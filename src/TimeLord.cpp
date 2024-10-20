/*
 * TimeLord.cpp
 *
 *  Created on: Dec 6, 2021
 *      Author: mad
 */

#include <mmx/TimeLord.h>
#include <mmx/ProofOfTime.hxx>
#include <mmx/WalletClient.hxx>

#include <vnx/vnx.h>

#include <sha256_ni.h>
#include <sha256_arm.h>


namespace mmx {

TimeLord::TimeLord(const std::string& _vnx_name)
	:	TimeLordBase(_vnx_name)
{
}

void TimeLord::init()
{
	subscribe(input_infuse, 1000);
	subscribe(input_request, 1000);

	vnx::open_pipe(vnx_name, this, 1000);
}

void TimeLord::main()
{
	if(enable_reward) {
		if(!reward_addr) {
			try {
				WalletClient wallet(wallet_server);
				for(const auto& entry : wallet.get_all_accounts()) {
					if(entry.address) {
						reward_addr = entry.address;
						break;
					}
				}
				if(!reward_addr) {
					throw std::logic_error("no wallet available");
				}
			}
			catch(const std::exception& ex) {
				log(WARN) << "Failed to get reward address from wallet: " << ex.what();
			}
		}
		if(reward_addr) {
			log(INFO) << "Reward address: " << reward_addr->to_string();
		} else {
			enable_reward = false;
		}
	} else {
		log(INFO) << "Reward is disabled";
	}
	{
		vnx::File file(storage_path + "timelord_sk.dat");
		if(file.exists()) {
			try {
				file.open("rb");
				vnx::read_generic(file.in, timelord_sk);
				file.close();
			}
			catch(const std::exception& ex) {
				log(WARN) << "Failed to read key from file: " << ex.what();
			}
		}
		if(timelord_sk == skey_t()) {
			timelord_sk = hash_t::random();
			try {
				file.open("wb");
				vnx::write_generic(file.out, timelord_sk);
				file.close();
			}
			catch(const std::exception& ex) {
				log(WARN) << "Failed to write key to file: " << ex.what();
			}
		}
		timelord_key = pubkey_t::from_skey(timelord_sk);
		log(DEBUG) << "Timelord Key: " << timelord_key;
	}

	set_timer_millis(10000, std::bind(&TimeLord::print_info, this));

	Super::main();

	stop_vdf();
}

void TimeLord::start_vdf(vdf_point_t begin)
{
	if(!is_running) {
		is_running = true;
		latest_point = nullptr;
		last_restart = vnx::get_time_micros();
		log(INFO) << "Started VDF at " << begin.num_iters;
		vdf_thread = std::thread(&TimeLord::vdf_loop, this, begin);
	}
}

void TimeLord::stop_vdf()
{
	{
		std::lock_guard<std::recursive_mutex> lock(mutex);
		is_running = false;
	}
	if(vdf_thread.joinable()) {
		vdf_thread.join();
	}
	clear_history();
}

void TimeLord::clear_history()
{
	history.clear();
	infuse_history[0].clear();
	infuse_history[1].clear();
	latest_point = nullptr;
}

void TimeLord::handle(std::shared_ptr<const TimeInfusion> value)
{
	if(value->chain > 1) {
		return;
	}
	const auto& values = value->values;

	std::lock_guard<std::recursive_mutex> lock(mutex);

	auto& map = infuse[value->chain];
	for(const auto& entry : values) {
		const bool passed = latest_point && entry.first < latest_point->num_iters;
		
		auto iter = map.find(entry.first);
		if(iter != map.end()) {
			if(entry.second != iter->second && passed) {
				log(WARN) << "Infusion value at " << entry.first << " changed, restarting ...";
				add_task([this]{
					stop_vdf();
				});
			}
		} else {
			if(passed) {
				log(WARN) << "Missed infusion point at " << entry.first << " iterations, restarting ...";
				add_task([this]{
					stop_vdf();
				});
			}
			log(DEBUG) << "Infusing at " << entry.first << " on chain " << value->chain << ": " << entry.second;
		}
	}
	if(!values.empty()) {
		map.erase(map.lower_bound(values.begin()->first), map.end());
	}
	map.insert(values.begin(), values.end());
}

void TimeLord::handle(std::shared_ptr<const IntervalRequest> request)
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	segment_iters = (request->end - request->begin) / request->num_segments;

	if(request->has_start)
	{
		vdf_point_t begin;
		begin.num_iters = request->begin;
		begin.output = request->start_values;
		{
			auto iter = history.find(request->begin);
			if((iter != history.end() && iter->second.output != request->start_values)
				|| (iter == history.end() && latest_point && latest_point->num_iters > request->begin))
			{
				log(WARN) << "Our VDF forked from the network, restarting ...";
				add_task([this]{
					stop_vdf();
				});
			}
		}
		if(!is_running) {
			start_vdf(begin);
		}
		else if(!latest_point || begin.num_iters > latest_point->num_iters) {
			// another timelord is faster
			const auto now = vnx::get_time_micros();
			if((now - last_restart) / 1000 > restart_holdoff) {
				last_restart = now;
				latest_point = std::make_shared<vdf_point_t>(begin);
			}
		}
	}

	if(request->end > request->begin) {
		request_t req;
		req.height = request->height;
		req.segment_iters = segment_iters;
		pending.emplace(std::make_pair(request->end, request->begin), req);
	}
	update();
}

void TimeLord::update()
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	for(auto iter = pending.begin(); iter != pending.end();)
	{
		const auto iters_end = iter->first.first;
		const auto iters_begin = iter->first.second;

		auto end = history.find(iters_end);
		if(end != history.end())
		{
			auto begin = history.find(iters_begin);
			if(begin != history.end())
			{
				auto proof = ProofOfTime::create();
				proof->start = iters_begin;
				proof->height = iter->second.height;
				proof->input = begin->second.output;

				for(uint32_t k = 0; k < 2; ++k) {
					auto iter = infuse_history[k].find(iters_begin);
					if(iter != infuse_history[k].end()) {
						proof->infuse[k] = iter->second;
					}
				}

				end++;
				begin++;
				auto prev_iters = iters_begin;
				for(auto iter = begin; iter != end; ++iter) {
					time_segment_t seg;
					seg.num_iters = iter->first - prev_iters;
					seg.output = iter->second.output;
					prev_iters = iter->first;
					proof->segments.push_back(seg);

					if(enable_reward) {
						proof->reward_segments.push_back(iter->second.reward_output);
					}
				}
				if(enable_reward) {
					proof->reward_addr = reward_addr;
				}
				proof->timelord_key = timelord_key;
				proof->hash = proof->calc_hash().first;
				proof->timelord_sig = signature_t::sign(timelord_sk, proof->hash);
				proof->content_hash = proof->calc_hash().second;

				log(DEBUG) << "Created VDF for height " << proof->height << " with " << proof->segments.size() << " segments";

				publish(proof, output_proofs);

				iter = pending.erase(iter);
				continue;
			}
		}
		if(!history.empty() && history.upper_bound(iters_begin) == history.begin())
		{
			iter = pending.erase(iter);
			continue;
		}
		iter++;
	}
}

void TimeLord::vdf_loop(vdf_point_t point)
{
	bool do_notify = false;
	vnx::ThreadPool threads(enable_reward ? 3 : 2);

	while(vnx_do_run())
	{
		const auto time_begin = vnx::get_wall_time_micros();

		uint64_t next_target = 0;
		uint64_t checkpoint_iters = 0;
		{
			std::lock_guard<std::recursive_mutex> lock(mutex);

			checkpoint_iters = segment_iters;

			if(!is_running) {
				break;
			}
			if(latest_point && latest_point->num_iters > point.num_iters)
			{
				point = *latest_point;
				log(INFO) << "Restarted VDF at " << point.num_iters;
			}
			else {
				if(!latest_point) {
					latest_point = std::make_shared<vdf_point_t>();
				}
				*latest_point = point;
			}
			history.emplace(point.num_iters, point);

			// purge history
			while(history.size() > max_history) {
				history.erase(history.begin());
			}

			for(uint32_t k = 0; k < 2; ++k)
			{
				{
					// apply infusions
					auto iter = infuse[k].find(point.num_iters);
					if(iter != infuse[k].end()) {
						if(enable_reward && k == 0) {
							point.reward_output = hash_t(point.output[k] + (*reward_addr));
						}
						point.output[k] = hash_t(point.output[k] + iter->second);
						infuse_history[k].insert(*iter);
					}
				}
				{
					// check for upcoming infusion point
					auto iter = infuse[k].upper_bound(point.num_iters);
					if(iter != infuse[k].end()) {
						if(!next_target || iter->first < next_target) {
							next_target = iter->first;
						}
					}
				}
				// purge history
				while(infuse_history[k].size() > 1000) {
					infuse_history[k].erase(infuse_history[k].begin());
				}
			}
			{
				// check for upcoming boundary point
				auto iter = pending.upper_bound(std::make_pair(point.num_iters, point.num_iters));
				if(iter != pending.end()) {
					const auto iters_end = iter->first.first;
					if(!next_target || iters_end < next_target) {
						next_target = iters_end;
					}
					checkpoint_iters = iter->second.segment_iters;
				}
				if(iter != pending.begin()) {
					do_notify = true;
				}
			}
			if(do_notify) {
				add_task(std::bind(&TimeLord::update, this));
			}
		}
		do_notify = false;

		const auto checkpoint = point.num_iters + checkpoint_iters;

		if(next_target <= point.num_iters) {
			next_target = checkpoint;
		}
		else if(next_target <= checkpoint) {
			do_notify = true;
		}
		else {
			next_target = checkpoint;
		}
		const auto num_iters = next_target - point.num_iters;

		for(auto& hash : point.output) {
			threads.add_task([num_iters, &hash]() {
				hash = compute(hash, num_iters);
			});
		}
		if(enable_reward) {
			threads.add_task([num_iters, &point]() {
				point.reward_output = compute(point.reward_output, num_iters);
			});
		}
		threads.sync();

		point.num_iters += num_iters;

		const auto time_end = vnx::get_wall_time_micros();

		if(time_end > time_begin && num_iters > checkpoint_iters / 2)
		{
			// update estimated number of iterations per second
			const auto speed = (num_iters * 1000000) / (time_end - time_begin);
			avg_iters_per_sec = (avg_iters_per_sec * 255 + speed) / 256;
		}
	}
	threads.close();

	log(INFO) << "Stopped VDF";
}

hash_t TimeLord::compute(const hash_t& input, const uint64_t num_iters)
{
	static bool have_sha_ni = sha256_ni_available();
	static bool have_sha_arm = sha256_arm_available();

	hash_t hash = input;
	if(have_sha_ni) {
		recursive_sha256_ni(hash.data(), num_iters);
	} else if(have_sha_arm) {
		recursive_sha256_arm(hash.data(), num_iters);
	} else {
		for(uint64_t i = 0; i < num_iters; ++i) {
			hash = hash_t(hash.bytes);
		}
	}
	return hash;
}

void TimeLord::print_info()
{
	if(is_running) {
		log(INFO) << double(avg_iters_per_sec) / 1e6 << " MH/s";
	}
	vnx::open_flow(vnx::get_pipe(node_server), vnx::get_pipe(vnx_name));
}


} // mmx

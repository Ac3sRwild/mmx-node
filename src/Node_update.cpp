/*
 * Node_update.cpp
 *
 *  Created on: Jan 20, 2022
 *      Author: mad
 */

#include <mmx/Node.h>
#include <mmx/FarmerClient.hxx>
#include <mmx/TimeInfusion.hxx>
#include <mmx/IntervalRequest.hxx>
#include <mmx/ProofOfSpaceOG.hxx>
#include <mmx/ProofOfSpaceNFT.hxx>
#include <mmx/ProofOfStake.hxx>
#include <mmx/contract/VirtualPlot.hxx>
#include <mmx/operation/Execute.hxx>
#include <mmx/operation/Deposit.hxx>
#include <mmx/utils.h>

#include <vnx/vnx.h>
#include <random>


namespace mmx {

void Node::verify_vdfs()
{
	for(auto iter = pending_vdfs.begin(); iter != pending_vdfs.end();) {
		const auto& proof = iter->second;
		if(!vdf_verify_pending.count(proof->height)) {
			add_task([this, proof]() {
				handle(proof);
			});
			iter = pending_vdfs.erase(iter);
		} else {
			iter++;
		}
	}
}

void Node::verify_proofs()
{
	std::vector<std::shared_ptr<const ProofResponse>> try_again;

	for(const auto& response : pending_proofs) {
		try {
			if(!verify(response)) {
				try_again.push_back(response);
			}
		} catch(const std::exception& ex) {
			log(WARN) << "Got invalid proof for height " << (response->request ? response->request->height : 0) << ": " << ex.what();
		} catch(...) {
			// ignore
		}
	}
	pending_proofs = std::move(try_again);
}

void Node::pre_validate_blocks()
{
	for(const auto& fork : pending_forks) {
		threads->add_task([this, fork]() {
			const auto& block = fork->block;
			try {
				if(!block->is_valid()) {
					throw std::logic_error("invalid block");
				}
				// need to verify farmer_sig before adding to fork tree
				block->validate();
			}
			catch(const std::exception& ex) {
				fork->is_invalid = true;
				log(WARN) << "Pre-validation failed for a block at height " << block->height << ": " << ex.what();
			}
			catch(...) {
				fork->is_invalid = true;
			}
		});
	}
	threads->sync();

	const auto list = std::move(pending_forks);
	pending_forks.clear();

	for(const auto& fork : list) {
		if(!fork->is_invalid) {
			add_fork(fork);
		}
	}
}

void Node::verify_block_proofs()
{
	std::mutex mutex;
	const auto root = get_root();
	for(const auto& entry : fork_index) {
		const auto& fork = entry.second;
		if(fork->is_proof_verified) {
			continue;
		}
		const auto& block = fork->block;
		if(!fork->prev.lock()) {
			if(auto prev = find_fork(block->prev)) {
				fork->prev = prev;
			}
		}
		if(!fork->diff_block) {
			fork->diff_block = find_diff_header(block);
		}
		if(auto prev = fork->prev.lock()) {
			if(prev->is_invalid) {
				fork->is_invalid = true;
			}
			fork->is_connected = prev->is_connected;
		} else if(block->prev == root->hash) {
			fork->is_connected = true;
		}
		const auto prev = find_prev_header(block);
		if(!prev || fork->is_invalid || !fork->diff_block || !fork->is_connected) {
			continue;
		}
		if(auto point = find_vdf_point(block->height, prev->vdf_iters, block->vdf_iters, prev->vdf_output, block->vdf_output)) {
			fork->vdf_point = point;
			fork->is_vdf_verified = true;
		}
		if(fork->is_vdf_verified || !is_synced) {
			threads->add_task([this, fork, &mutex]() {
				const auto& block = fork->block;
				try {
					hash_t challenge;
					if(find_challenge(block, challenge))
					{
						verify_proof(fork, challenge);

						if(auto proof = block->proof) {
							std::lock_guard<std::mutex> lock(mutex);
							add_proof(block->height, challenge, proof, vnx::Hash64());
						}
					}
				} catch(const std::exception& ex) {
					fork->is_invalid = true;
					log(WARN) << "Proof verification failed for a block at height " << block->height << ": " << ex.what();
				} catch(...) {
					fork->is_invalid = true;
				}
			});
		}
	}
	threads->sync();
}

void Node::add_dummy_block(std::shared_ptr<const BlockHeader> prev)
{
	if(auto vdf_point = find_next_vdf_point(prev))
	{
		const auto diff_block = get_diff_header(prev, 1);

		auto block = Block::create();
		block->version = 0;
		block->prev = prev->hash;
		block->height = prev->height + 1;
		block->time_stamp = prev->time_stamp + params->block_interval_ms;
		block->time_diff = prev->time_diff;
		block->space_diff = calc_new_space_diff(params, prev->space_diff, params->score_threshold);
		block->vdf_iters = vdf_point->vdf_iters;
		block->vdf_output = vdf_point->output;
		block->weight = calc_block_weight(params, diff_block, block);
		block->total_weight = prev->total_weight + block->weight;
		block->netspace_ratio = prev->netspace_ratio;
		block->average_txfee = prev->average_txfee;
		block->next_base_reward = prev->next_base_reward;
		block->finalize();
		add_block(block);
	}
}

void Node::add_dummy_blocks(const uint32_t& height)
{
	const auto root = get_root();
	if(height == root->height + 1) {
		add_dummy_block(root);
	}
	const auto range = fork_index.equal_range(height - 1);
	for(auto iter = range.first; iter != range.second; ++iter) {
		add_dummy_block(iter->second->block);
	}
}

void Node::update()
{
	std::unique_lock lock(db_mutex);

	const auto time_begin = vnx::get_wall_time_millis();
	update_pending = false;

	verify_vdfs();

	verify_proofs();

	pre_validate_blocks();

	verify_block_proofs();

	const auto prev_peak = get_peak();
	std::shared_ptr<const BlockHeader> forked_at;

	// choose best fork
	while(vnx::do_run())
	{
		forked_at = nullptr;
		const auto best_fork = find_best_fork();

		if(!best_fork || best_fork->block->hash == state_hash) {
			break;	// no change
		}

		// verify and apply new fork
		try {
			forked_at = fork_to(best_fork);
		} catch(const std::exception& ex) {
			best_fork->is_invalid = true;
			log(WARN) << "Forking to height " << best_fork->block->height << " failed with: " << ex.what();
			continue;	// try again
		}
		break;
	}

	const auto peak = get_peak();
	if(!peak) {
		log(WARN) << "Have no peak!";
		return;
	}
	{
		// commit to disk
		const auto fork_line = get_fork_line();
		const auto commit_delay = is_synced || sync_retry ? params->commit_delay : max_fork_length;

		size_t num_empty = 0;
		for(const auto& fork : fork_line) {
			if(!fork->block->farmer_sig) {
				num_empty++;
			}
		}
		if(num_empty < fork_line.size() / 2)
		{
			for(size_t i = 0; i + commit_delay < fork_line.size(); ++i)
			{
				const auto& fork = fork_line[i];
				const auto& block = fork->block;
				if(!fork->is_vdf_verified) {
					break;	// wait for VDF verify
				}
				commit(block);
			}
		}
	}
	const auto root = get_root();
	const auto now_ms = vnx::get_wall_time_millis();
	const auto elapsed = (now_ms - time_begin) / 1e3;

	if(!prev_peak || peak->hash != prev_peak->hash)
	{
		if(auto fork = find_fork(peak->hash))
		{
			const auto proof = fork->block->proof;
			std::stringstream msg;
			msg << u8"\U0001F4BE New peak at height " << peak->height << " with score ";
			if(proof) {
				msg << proof->score;
			} else {
				msg << "N/A";
			}
			if(is_synced) {
				if(forked_at) {
					msg << ", forked at " << forked_at->height;
				}
				if(auto vdf_point = fork->vdf_point) {
					msg << ", delay " << (fork->recv_time - vdf_point->recv_time) / 1000 / 1e3 << " sec";
				}
			} else {
				msg << ", " << sync_pending.size() << " pending";
				if(auto count = vdf_threads->get_num_pending_total()) {
					msg << ", " << count << " vdf checks";
				}
			}
			msg << ", took " << elapsed << " sec";
			log(proof || forked_at || !is_synced ? INFO : DEBUG) << msg.str();
		}
		stuck_timer->reset();
	}

	if(!is_synced && sync_peak && sync_pending.empty() && !vdf_threads->get_num_pending_total())
	{
		if(sync_retry < num_sync_retries) {
			if(now_ms - sync_finish_ms > params->block_interval_ms / 2) {
				log(INFO) << "Reached sync peak at height " << *sync_peak - 1;
				sync_pos = *sync_peak;
				sync_peak = nullptr;
				sync_finish_ms = now_ms;
				sync_retry++;
			}
		} else {
			is_synced = true;
			on_sync_done(peak->height);
		}
	}
	if(!is_synced) {
		sync_more();
		update_timer->reset();
		return;
	}
	{
		// publish time infusions for VDF 0
		auto infuse = TimeInfusion::create();
		infuse->chain = 0;
		auto vdf_iters = peak->vdf_iters;
		for(uint32_t i = 0; i <= params->infuse_delay; ++i)
		{
			if(auto prev = find_prev_header(peak, params->infuse_delay - i))
			{
				infuse->values[vdf_iters] = prev->hash;
			}
			auto diff_block = get_diff_header(peak, i + 1);
			vdf_iters += diff_block->time_diff * params->time_diff_constant;
		}
		publish(infuse, output_timelord_infuse);
	}
	{
		// publish next time infusion for VDF 1
		uint32_t height = peak->height;
		height -= (height % params->challenge_interval);
		if(auto prev = find_prev_header(peak, peak->height - height))
		{
			if(prev->height >= params->challenge_interval)
			{
				if(auto diff_block = find_prev_header(prev, params->challenge_interval))
				{
					auto infuse = TimeInfusion::create();
					infuse->chain = 1;
					const auto vdf_iters = prev->vdf_iters + diff_block->time_diff * params->time_diff_constant * params->infuse_delay;
					infuse->values[vdf_iters] = prev->hash;
					publish(infuse, output_timelord_infuse);
				}
			}
		}
	}
	{
		// request next VDF proofs
		auto vdf_iters = peak->vdf_iters;
		for(uint32_t i = 0; i < params->infuse_delay; ++i)
		{
			auto request = IntervalRequest::create();
			request->begin = vdf_iters;
			if(i == 0) {
				request->has_start = true;
				request->start_values = peak->vdf_output;
			}
			auto diff_block = get_diff_header(peak, i + 1);
			vdf_iters += diff_block->time_diff * params->time_diff_constant;
			request->end = vdf_iters;
			request->height = peak->height + i + 1;
			request->num_segments = params->num_vdf_segments;
			publish(request, output_interval_request);
		}
	}

	// try to make a block
	for(uint32_t depth = 0; depth <= 1u && depth <= peak->height; ++depth)
	{
		const auto height = peak->height - depth;
		// find best peak at this height
		std::shared_ptr<const BlockHeader> prev;
		if(height < root->height) {
			break;
		} else if(depth == 0) {
			prev = peak;
		} else if(height == root->height) {
			prev = root;
		} else if(auto fork = find_best_fork(height)) {
			prev = fork->block;
		}
		hash_t challenge;
		if(find_challenge(prev, challenge, 1))
		{
			const auto proof_list = find_proof(challenge);

			// Note: proof_list already limited to max_blocks_per_height
			for(size_t proof_index = 0; proof_index < proof_list.size(); ++proof_index)
			{
				const auto& proof = proof_list[proof_index];
				// check if it's our proof and farmer is still alive
				if(vnx::get_pipe(proof.farmer_mac))
				{
					const auto key = std::make_pair(prev->height + 1, proof.hash);
					if(!created_blocks.count(key))
					{
						if(auto vdf_point = find_next_vdf_point(prev)) {
							try {
								// make a full block only if (we extend peak or replace a dummy peak or replace a weaker peak) and (we got best score)
								const bool is_better = (depth == 1 && (!peak->proof || !peak->tx_count || proof.proof->score < peak->proof->score));
								const bool full_block = (depth == 0 || is_better) && proof_index == 0;
								if(auto block = make_block(prev, vdf_point, proof, full_block)) {
									created_blocks[key] = block->hash;
									add_block(block);
								}
							} catch(const std::exception& ex) {
								log(WARN) << "Failed to create block at height " << key.first << ": " << ex.what();
							}
							// revert back to peak
							fork_to(peak->hash);
						}
					}
				}
			}
		}
	}

	// publish challenges
	for(uint32_t i = 0; i <= params->challenge_delay; ++i)
	{
		if(auto vdf_block = find_prev_header(peak, params->challenge_delay - i))
		{
			auto value = Challenge::create();
			value->height = peak->height + i;
			value->challenge = vdf_block->vdf_output[1];
			const auto diff_block = get_diff_header(peak, i);
			value->space_diff = diff_block->space_diff;
			value->diff_block_hash = diff_block->hash;
			value->max_delay = 1 + i;
			publish(value, output_challenges);
		}
	}
}

void Node::on_sync_done(const uint32_t height)
{
	log(INFO) << "Finished sync at height " << height;
	update_control();
}

bool Node::tx_pool_update(const tx_pool_t& entry, const bool force_add)
{
	if(const auto& tx = entry.tx) {
		if(tx->sender) {
			const auto& sender = *tx->sender;
			const auto iter = tx_pool.find(tx->id);
			const auto fees = tx_pool_fees.find(sender);
			auto new_total = (fees != tx_pool_fees.end()) ? fees->second : 0;
			if(iter != tx_pool.end()) {
				new_total -= iter->second.fee;
			}
			new_total += entry.fee;

			if(!force_add && new_total > get_balance(sender, addr_t())) {
				return false;
			}
			if(fees != tx_pool_fees.end()) {
				fees->second = new_total;
			} else {
				tx_pool_fees[sender] = new_total;
			}
			if(iter != tx_pool.end()) {
				iter->second = entry;
			} else {
				tx_pool[tx->id] = entry;
			}
			return true;
		}
	}
	return false;
}

void Node::tx_pool_erase(const hash_t& txid)
{
	const auto iter = tx_pool.find(txid);
	if(iter != tx_pool.end()) {
		if(const auto& tx = iter->second.tx) {
			if(const auto& sender = tx->sender) {
				const auto iter2 = tx_pool_fees.find(*sender);
				if(iter2 != tx_pool_fees.end()) {
					if((iter2->second -= iter->second.fee) == 0) {
						tx_pool_fees.erase(iter2);
					}
				}
			}
		}
		tx_pool.erase(iter);
	}
}

void Node::purge_tx_pool()
{
	const auto time_begin = vnx::get_wall_time_millis();

	std::vector<tx_pool_t> all_tx;
	all_tx.reserve(tx_pool.size());
	for(const auto& entry : tx_pool) {
		all_tx.push_back(entry.second);
	}

	// sort transactions by fee ratio
	std::sort(all_tx.begin(), all_tx.end(),
		[](const tx_pool_t& lhs, const tx_pool_t& rhs) -> bool {
			const auto L = lhs.tx->fee_ratio;
			const auto R = rhs.tx->fee_ratio;
			return (L == R) ? lhs.luck < rhs.luck : L > R;
		});

	size_t num_purged = 0;
	uint64_t total_pool_size = 0;
	std::unordered_map<addr_t, std::pair<uint64_t, uint64_t>> sender_map;	// [sender => [balance, total fee]]

	const uint64_t max_pool_size = uint64_t(tx_pool_limit) * params->max_block_size;

	// purge transactions from pool if overflowing
	for(const auto& entry : all_tx) {
		const auto& tx = entry.tx;
		bool fee_overspend = false;
		if(tx->sender) {
			const auto sender = *tx->sender;
			const auto iter = sender_map.emplace(sender, std::make_pair(0, 0));
			auto& balance = iter.first->second.first;
			auto& total_fee = iter.first->second.second;
			if(iter.second) {
				balance = get_balance(sender, addr_t());
			}
			total_fee += entry.fee;
			fee_overspend = total_fee > balance;
		}
		if(!fee_overspend) {
			total_pool_size += tx->static_cost;
		}
		if(total_pool_size > max_pool_size || fee_overspend) {
			tx_pool_erase(tx->id);
			num_purged++;
		} else {
			min_pool_fee_ratio = tx->fee_ratio;
		}
	}
	if(total_pool_size < 9 * max_pool_size / 10) {
		min_pool_fee_ratio = 0;
	}
	if(total_pool_size || num_purged) {
		log(INFO) << uint64_t((total_pool_size * 10000) / max_pool_size) / 100. << " % mem pool, "
				<< min_pool_fee_ratio / 1024. << " min fee ratio, " << num_purged << " purged, took "
				<< (vnx::get_wall_time_millis() - time_begin) / 1e3 << " sec";
	}
}

void Node::validate_new()
{
	const auto peak = get_peak();
	if(!peak || !is_synced) {
		return;
	}
	std::default_random_engine luck_gen(vnx::rand64());

	// select non-overlapping set
	std::vector<tx_pool_t> tx_list;
	std::unordered_set<hash_t> tx_set;
	for(const auto& entry : tx_queue) {
		if(const auto& tx = entry.second) {
			if(tx_set.insert(tx->id).second) {
				tx_pool_t tmp;
				tmp.tx = tx;
				tmp.luck = luck_gen();
				tx_list.push_back(tmp);
			}
		}
	}

	auto context = new_exec_context(peak->height + 1);

	// prepare synchronization
	for(const auto& entry : tx_list) {
		prepare_context(context, entry.tx);
	}

	// verify transactions in parallel
	for(auto& entry : tx_list) {
		threads->add_task([this, &entry, context]() {
			entry.is_valid = false;
			auto& tx = entry.tx;
			if(tx->exec_result) {
				auto tmp = vnx::clone(tx);
				tmp->reset(params);
				tx = tmp;
			}
			context->wait(tx->id);
			try {
				if(auto result = validate(tx, context)) {
					entry.cost = result->total_cost;
					entry.fee = result->total_fee;
					entry.is_valid = true;
				}
			} catch(const std::exception& ex) {
				if(show_warnings) {
					log(WARN) << "TX validation failed with: " << ex.what() << " (" << tx->id << ")";
				}
			} catch(...) {
				// ignore
			}
			context->signal(tx->id);
		});
	}
	threads->sync();

	// update tx pool
	for(const auto& entry : tx_list) {
		const auto& tx = entry.tx;
		if(entry.is_valid) {
			if(tx_pool_update(entry)) {
				publish(tx, output_verified_transactions);
			}
		}
		tx_queue.erase(tx->content_hash);
	}
}

std::vector<Node::tx_pool_t> Node::validate_for_block()
{
	const auto peak = get_peak();
	if(!peak) {
		return {};
	}
	std::vector<tx_pool_t> all_tx;
	all_tx.reserve(tx_pool.size());
	for(const auto& entry : tx_pool) {
		all_tx.push_back(entry.second);
	}

	// sort transactions by fee ratio
	std::sort(all_tx.begin(), all_tx.end(),
		[](const tx_pool_t& lhs, const tx_pool_t& rhs) -> bool {
			return lhs.tx->fee_ratio > rhs.tx->fee_ratio;
		});

	auto context = new_exec_context(peak->height + 1);

	std::vector<tx_pool_t> tx_list;
	uint64_t total_verify_cost = 0;

	// select transactions to verify
	for(const auto& entry : all_tx) {
		if(check_tx_inclusion(entry.tx->id, context->height)) {
			if(total_verify_cost + entry.cost <= params->max_block_cost) {
				tx_list.push_back(entry);
				total_verify_cost += entry.cost;
			}
		}
	}

	// prepare synchronization
	for(const auto& entry : tx_list) {
		prepare_context(context, entry.tx);
	}

	// verify transactions in parallel
	for(auto& entry : tx_list) {
		threads->add_task([this, &entry, context]() {
			entry.is_valid = false;
			auto& tx = entry.tx;
			if(tx->exec_result) {
				auto tmp = vnx::clone(tx);
				tmp->reset(params);
				tx = tmp;
			}
			context->wait(tx->id);
			try {
				auto result = validate(tx, context);
				if(!result) {
					throw std::logic_error("!result");
				}
				auto tmp = vnx::clone(tx);
				tmp->update(*result, params);
				tx = tmp;
				entry.cost = result->total_cost;
				entry.fee = result->total_fee;
				entry.is_valid = true;
			} catch(const std::exception& ex) {
				if(show_warnings) {
					log(WARN) << "TX validation failed with: " << ex.what() << " (" << tx->id << ")";
				}
			} catch(...) {
				// ignore
			}
			context->signal(tx->id);
		});
	}
	threads->sync();

	uint64_t total_cost = 0;
	uint64_t static_cost = 0;
	std::vector<tx_pool_t> result;
	balance_cache_t balance_cache(&balance_table);

	// select final set of transactions
	for(auto& entry : tx_list)
	{
		if(!entry.is_valid) {
			tx_pool_erase(entry.tx->id);
			continue;
		}
		const auto tx = entry.tx;

		if(static_cost + tx->static_cost > params->max_block_size || total_cost + entry.cost > params->max_block_cost) {
			continue;
		}
		bool passed = true;
		balance_cache_t tmp_cache(&balance_cache);
		{
			const auto balance = tmp_cache.find(*tx->sender, addr_t());
			if(balance && entry.fee <= *balance) {
				*balance -= entry.fee;
			} else {
				passed = false;
			}
		}
		if(!tx->exec_result->did_fail) {
			for(const auto& in : tx->inputs) {
				const auto balance = tmp_cache.find(in.address, in.contract);
				if(balance && in.amount <= *balance) {
					*balance -= in.amount;
				} else {
					passed = false;
				}
			}
		}
		if(!passed) {
			continue;
		}
		balance_cache.apply(tmp_cache);

		total_cost += entry.cost;
		static_cost += tx->static_cost;
		result.push_back(entry);
	}

	const uint32_t N = params->min_fee_ratio.size();
	if(N == 0) {
		return result;
	}
	std::vector<uint64_t> band_avail(N);
	for(uint64_t i = 0; i < N; ++i) {
		band_avail[i] = ((i + 1) * params->max_block_size) / N - (i * params->max_block_size) / N;
	}
	uint32_t i = N - 1;
	std::vector<tx_pool_t> out;
	for(const auto& entry : result) {
		const auto& tx = entry.tx;
		while(i && (!band_avail[i] || tx->fee_ratio < params->min_fee_ratio[i] * 1024)) {
			i--;
		}
		if(tx->static_cost > band_avail[i]) {
			if(i) {
				// we assume band size is always >= max_tx_cost (so band_avail never goes negative here)
				band_avail[i - 1] -= tx->static_cost - band_avail[i];
				band_avail[i] = 0;
			} else {
				continue;	// no room left for this tx
			}
		} else {
			band_avail[i] -= tx->static_cost;
		}
		out.push_back(entry);
	}
	return out;
}

std::shared_ptr<const Block> Node::make_block(std::shared_ptr<const BlockHeader> prev, std::shared_ptr<const VDF_Point> vdf_point, const proof_data_t& proof, const bool full_block)
{
	const auto time_begin = vnx::get_wall_time_millis();

	// reset state to previous block
	fork_to(prev->hash);

	auto block = Block::create();
	block->prev = prev->hash;
	block->height = prev->height + 1;
	block->time_diff = prev->time_diff;
	block->space_diff = prev->space_diff;
	block->vdf_iters = vdf_point->vdf_iters;
	block->vdf_output = vdf_point->output;
	block->proof = proof.proof;
	block->netspace_ratio = calc_new_netspace_ratio(
			params, prev->netspace_ratio, bool(std::dynamic_pointer_cast<const ProofOfSpaceOG>(block->proof)));
	block->vdf_reward_addr = vdf_point->reward_addr;

	if(auto stake = std::dynamic_pointer_cast<const ProofOfStake>(block->proof)) {
		if(auto plot = get_contract_as<contract::VirtualPlot>(stake->plot_id)) {
			block->reward_addr = plot->reward_address;
		}
	} else if(auto nft_proof = std::dynamic_pointer_cast<const ProofOfSpaceNFT>(block->proof)) {
		block->reward_addr = nft_proof->contract;
	}

	uint64_t total_fees = 0;
	if(full_block && block->height >= params->transaction_activation)
	{
		const auto tx_list = validate_for_block();
		// select transactions
		for(const auto& entry : tx_list) {
			block->tx_list.push_back(entry.tx);
			total_fees += entry.fee;
		}
	}
	block->average_txfee = calc_new_average_txfee(params, prev->average_txfee, total_fees);

	const auto prev_fork = find_fork(prev->hash);

	// set time stamp
	if(auto point = prev_fork->vdf_point) {
		auto delta_ms = (vdf_point->recv_time - point->recv_time) / 1000;
		delta_ms = std::min(delta_ms, params->block_interval_ms * 2);
		delta_ms = std::max(delta_ms, params->block_interval_ms / 2);
		block->time_stamp = prev->time_stamp + delta_ms;
	} else {
		block->time_stamp = prev->time_stamp + params->block_interval_ms;
	}

	// set new time difficulty
	if(auto fork = find_prev_fork(prev_fork, params->infuse_delay))
	{
		if(auto point = fork->vdf_point) {
			const int64_t delta_ms = (vdf_point->recv_time - point->recv_time) / (params->infuse_delay + 1) / 1000;
			if(delta_ms > 0) {
				const double gain = 0.1;
				if(auto diff_block = fork->diff_block) {
					auto new_diff = (double(params->block_interval_ms) * diff_block->time_diff) / delta_ms;
					new_diff = prev->time_diff * (1 - gain) + new_diff * gain;
					block->time_diff = std::max<int64_t>(new_diff + 0.5, 1);
				}
			}
		}
	}
	{
		// limit time diff update
		const auto max_update = std::max<uint64_t>(prev->time_diff >> params->max_diff_adjust, 1);
		block->time_diff = std::min(block->time_diff, prev->time_diff + max_update);
		block->time_diff = std::max(block->time_diff, prev->time_diff - max_update);
	}
	block->space_diff = calc_new_space_diff(params, prev->space_diff, block->proof->score);
	{
		const auto diff_block = get_diff_header(prev, 1);
		block->weight = calc_block_weight(params, diff_block, block);
		block->total_weight = prev->total_weight + block->weight;
	}
	block->reward_amount = calc_block_reward(block, total_fees);
	block->next_base_reward = calc_next_base_reward(params, prev->next_base_reward, prev->reward_vote);
	block->reward_vote = reward_vote;
	block->finalize();

	FarmerClient farmer(proof.farmer_mac);
	const auto result = farmer.sign_block(block);

	if(!result) {
		throw std::logic_error("farmer refused to sign block");
	}
	block->BlockHeader::operator=(*result);

	const auto elapsed = (vnx::get_wall_time_millis() - time_begin) / 1e3;
	log(INFO) << u8"\U0001F911 Created block at height " << block->height << " with: ntx = "
			<< (full_block ? std::to_string(block->tx_list.size()) : "dummy")
			<< ", score = " << block->proof->score << ", reward = " << to_value(block->reward_amount, params) << " MMX"
			<< ", fees = " << to_value(total_fees, params) << " MMX" << ", took " << elapsed << " sec";
	return block;
}


} // mmx

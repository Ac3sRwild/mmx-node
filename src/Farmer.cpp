/*
 * Farmer.cpp
 *
 *  Created on: Dec 12, 2021
 *      Author: mad
 */

#include <mmx/Farmer.h>
#include <mmx/Transaction.hxx>
#include <mmx/WalletClient.hxx>


namespace mmx {

Farmer::Farmer(const std::string& _vnx_name)
	:	FarmerBase(_vnx_name)
{
}

void Farmer::init()
{
	vnx::open_pipe(vnx_name, this, 1000);
	vnx::open_pipe(vnx_get_id(), this, 1000);

	subscribe(input_info, 1000);
}

void Farmer::main()
{
	if(reward_addr) {
		log(INFO) << "Reward address: " << reward_addr->to_string();
	}

	set_timer_millis(10000, std::bind(&Farmer::update, this));

	update();

	Super::main();
}

vnx::Hash64 Farmer::get_mac_addr() const
{
	return vnx_get_id();
}

std::shared_ptr<const FarmInfo> Farmer::get_farm_info() const
{
	auto info = FarmInfo::create();
	for(const auto& entry : info_map) {
		if(auto value = std::dynamic_pointer_cast<const FarmInfo>(entry.second->value)) {
			for(const auto& entry : value->plot_count) {
				info->plot_count[entry.first] += entry.second;
			}
			info->plot_dirs.insert(info->plot_dirs.end(), value->plot_dirs.begin(), value->plot_dirs.end());
			info->total_bytes += value->total_bytes;
		}
	}
	return info;
}

void Farmer::update()
{
	vnx::open_flow(vnx::get_pipe(node_server), vnx::get_pipe(vnx_get_id()));

	WalletClient wallet(wallet_server);
	try {
		for(auto keys : wallet.get_all_farmer_keys()) {
			if(keys) {
				if(key_map.emplace(keys->farmer_public_key, keys->farmer_private_key).second) {
					log(INFO) << "Got Farmer Key: " << keys->farmer_public_key;
				}
			}
		}
	}
	catch(const std::exception& ex) {
		log(WARN) << "Failed to get keys from wallet: " << ex.what();
	}
	try {
		if(!reward_addr) {
			const auto accounts = wallet.get_all_accounts();
			if(accounts.empty()) {
				throw std::logic_error("no wallet available");
			}
			reward_addr = wallet.get_address(accounts.begin()->first, 0);
			log(INFO) << "Reward address: " << reward_addr->to_string();
		}
	} catch(const std::exception& ex) {
		log(WARN) << "Failed to get reward address from wallet: " << ex.what();
	}

	const auto now = vnx::get_sync_time_micros();
	for(auto iter = info_map.begin(); iter != info_map.end();) {
		if((now - iter->second->recv_time) / 1000000 > harvester_timeout) {
			iter = info_map.erase(iter);
		} else {
			iter++;
		}
	}
}

void Farmer::handle(std::shared_ptr<const FarmInfo> value)
{
	if(auto sample = vnx_sample) {
		info_map[sample->src_mac] = sample;
	}
}

skey_t Farmer::find_skey(const bls_pubkey_t& pubkey) const
{
	auto iter = key_map.find(pubkey);
	if(iter == key_map.end()) {
		throw std::logic_error("unknown farmer key: " + pubkey.to_string());
	}
	return iter->second;
}

bls_signature_t Farmer::sign_proof(std::shared_ptr<const ProofOfSpace> proof) const
{
	if(!proof) {
		throw std::logic_error("!proof");
	}
	return bls_signature_t::sign(find_skey(proof->farmer_key), proof->calc_hash());
}

std::shared_ptr<const BlockHeader>
Farmer::sign_block(std::shared_ptr<const BlockHeader> block, const uint64_t& reward_amount) const
{
	if(!block) {
		throw std::logic_error("!block");
	}
	if(!block->proof) {
		throw std::logic_error("!proof");
	}
	const auto farmer_sk = find_skey(block->proof->farmer_key);

	auto base = Transaction::create();
	// TODO: use random nonce to make block hash unpredictable in case of no tx
	base->nonce = block->height;
	base->salt = block->vdf_output[0];

	auto amount_left = reward_amount;
	if(project_addr && amount_left > 0)
	{
		tx_out_t out;
		out.address = *project_addr;
		out.amount = double(amount_left) * devfee_ratio;
		if(out.amount > 0) {
			amount_left -= out.amount;
			base->outputs.push_back(out);
		}
	}
	if(reward_addr && amount_left > 0)
	{
		tx_out_t out;
		out.address = *reward_addr;
		out.amount = amount_left;
		amount_left -= out.amount;
		base->outputs.push_back(out);
	}
	base->finalize();

	auto copy = vnx::clone(block);
	copy->tx_base = base;
	copy->hash = copy->calc_hash();
	copy->farmer_sig = bls_signature_t::sign(farmer_sk, copy->hash);
	return copy;
}


} // mmx

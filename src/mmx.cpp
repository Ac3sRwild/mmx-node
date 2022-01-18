/*
 * mmx.cpp
 *
 *  Created on: Dec 21, 2021
 *      Author: mad
 */

#include <mmx/NodeClient.hxx>
#include <mmx/RouterClient.hxx>
#include <mmx/WalletClient.hxx>
#include <mmx/FarmerClient.hxx>
#include <mmx/HarvesterClient.hxx>
#include <mmx/Contract.hxx>
#include <mmx/contract/NFT.hxx>
#include <mmx/contract/Token.hxx>
#include <mmx/KeyFile.hxx>
#include <mmx/secp256k1.hpp>
#include <mmx/hash_t.hpp>
#include <mmx/utils.h>

#include <vnx/vnx.h>
#include <vnx/Proxy.h>


void show_history(const std::vector<mmx::tx_entry_t>& history, mmx::NodeClient& node, std::shared_ptr<const mmx::ChainParams> params)
{
	std::unordered_map<mmx::addr_t, std::shared_ptr<const mmx::contract::Token>> token_map;
	for(const auto& entry : history) {
		std::cout << "[" << entry.height << "] ";
		switch(entry.type) {
			case mmx::tx_type_e::SEND:    std::cout << "SEND    "; break;
			case mmx::tx_type_e::RECEIVE: std::cout << "RECEIVE "; break;
			case mmx::tx_type_e::REWARD:  std::cout << "REWARD  "; break;
			default: std::cout << "????    "; break;
		}
		auto iter = token_map.find(entry.contract);
		if(iter == token_map.end()) {
			iter = token_map.emplace(entry.contract, std::dynamic_pointer_cast<const mmx::contract::Token>(node.get_contract(entry.contract))).first;
		}
		const auto token = iter->second;
		const auto decimals = token ? token->decimals : params->decimals;
		std::cout << entry.amount / pow(10, decimals) << " " << (token ? token->symbol : "MMX") << " (" << entry.amount << ") -> " << entry.address << std::endl;
	}
}


int main(int argc, char** argv)
{
	mmx::secp256k1_init();

	std::map<std::string, std::string> options;
	options["n"] = "node";
	options["f"] = "file";
	options["j"] = "index";
	options["a"] = "amount";
	options["s"] = "source";
	options["t"] = "target";
	options["x"] = "contract";
	options["node"] = "address";
	options["file"] = "path";
	options["index"] = "0..?";
	options["amount"] = "123.456";
	options["source"] = "address";
	options["target"] = "address";
	options["contract"] = "address";

	vnx::write_config("log_level", 2);

	vnx::init("mmx", argc, argv, options);

	std::string module;
	std::string command;
	std::string file_name;
	std::string source_addr;
	std::string target_addr;
	std::string contract_addr;
	int64_t index = 0;
	double amount = 0;
	vnx::read_config("$1", module);
	vnx::read_config("$2", command);
	vnx::read_config("file", file_name);
	vnx::read_config("index", index);
	vnx::read_config("amount", amount);
	vnx::read_config("source", source_addr);
	vnx::read_config("target", target_addr);
	vnx::read_config("contract", contract_addr);

	bool did_fail = false;
	auto params = mmx::get_params();

	mmx::NodeClient node("Node");

	mmx::addr_t source;
	mmx::addr_t target;
	mmx::addr_t contract;
	try {
		if(!source_addr.empty()) {
			source.from_string(source_addr);
		}
	} catch(std::exception& ex) {
		vnx::log_error() << "Invalid address: " << ex.what();
		goto failed;
	}
	try {
		if(!target_addr.empty()) {
			target.from_string(target_addr);
		}
	} catch(std::exception& ex) {
		vnx::log_error() << "Invalid address: " << ex.what();
		goto failed;
	}
	try {
		if(!contract_addr.empty()) {
			contract.from_string(contract_addr);
		}
	} catch(std::exception& ex) {
		vnx::log_error() << "Invalid contract: " << ex.what();
		goto failed;
	}

	try {
		if(module == "wallet")
		{
			std::string node_url = ":11335";
			vnx::read_config("node", node_url);

			if(command != "create") {
				vnx::Handle<vnx::Proxy> proxy = new vnx::Proxy("Proxy", vnx::Endpoint::from_url(node_url));
				proxy->forward_list = {"Wallet", "Node"};
				proxy.start_detached();
				try {
					params = node.get_params();
				} catch(...) {
					// ignore
				}
			}
			mmx::WalletClient wallet("Wallet");

			if(command == "show")
			{
				int num_addrs = 1;
				vnx::read_config("$3", num_addrs);

				std::vector<mmx::addr_t> nfts;
				for(const auto& entry : wallet.get_balances(index))
				{
					const auto contract = node.get_contract(entry.first);
					if(std::dynamic_pointer_cast<const mmx::contract::NFT>(contract)) {
						nfts.push_back(entry.first);
					} else {
						const auto token = std::dynamic_pointer_cast<const mmx::contract::Token>(contract);
						const auto decimals = token ? token->decimals : params->decimals;
						std::cout << "Balance: " << entry.second / pow(10, decimals) << " " << (token ? token->symbol : "MMX") << " (" << entry.second << ")" << std::endl;
					}
				}
				for(const auto& addr : nfts) {
					std::cout << "NFT: " << addr;
				}
				for(int i = 0; i < num_addrs; ++i) {
					std::cout << "Address[" << i << "]: " << wallet.get_address(index, i) << std::endl;
				}
			}
			else if(command == "keys")
			{
				if(auto keys = wallet.get_farmer_keys(index)) {
					std::cout << "Pool Public Key:   " << keys->pool_public_key << std::endl;
					std::cout << "Farmer Public Key: " << keys->farmer_public_key << std::endl;
				} else {
					vnx::log_error() << "Got no wallet!";
				}
			}
			else if(command == "get")
			{
				std::string subject;
				vnx::read_config("$3", subject);

				if(subject == "address")
				{
					int64_t offset = 0;
					vnx::read_config("$4", offset);

					std::cout << wallet.get_address(index, offset) << std::endl;
				}
				else if(subject == "amount")
				{
					std::cout << wallet.get_balance(index, contract) << std::endl;
				}
				else if(subject == "balance")
				{
					const auto token = std::dynamic_pointer_cast<const mmx::contract::Token>(node.get_contract(contract));
					std::cout << wallet.get_balance(index, contract) / pow(10, token ? token->decimals : params->decimals) << std::endl;
				}
				else if(subject == "seed")
				{
					std::cout << wallet.get_master_seed(index) << std::endl;
				}
				else {
					std::cerr << "Help: mmx wallet get [address | amount | balance | seed]" << std::endl;
				}
			}
			else if(command == "send")
			{
				const auto token = std::dynamic_pointer_cast<const mmx::contract::Token>(node.get_contract(contract));

				const auto decimals = token ? token->decimals : params->decimals;
				const int64_t mojo = amount * pow(10, decimals);
				if(amount <= 0 || mojo <= 0) {
					vnx::log_error() << "Invalid amount: " << amount << " (-a | --amount)";
					goto failed;
				}
				if(target == mmx::addr_t()) {
					vnx::log_error() << "Missing destination address argument: -t | --target";
					goto failed;
				}
				const auto txid = wallet.send(index, mojo, target, contract);
				std::cout << "Sent " << mojo / pow(10, decimals) << " " << (token ? token->symbol : "MMX") << " (" << mojo << ") to " << target << std::endl;
				std::cout << "Transaction ID: " << txid << std::endl;
			}
			else if(command == "send_from")
			{
				const auto token = std::dynamic_pointer_cast<const mmx::contract::Token>(node.get_contract(contract));

				const auto decimals = token ? token->decimals : params->decimals;
				const int64_t mojo = amount * pow(10, decimals);
				if(amount <= 0 || mojo <= 0) {
					vnx::log_error() << "Invalid amount: " << amount << " (-a | --amount)";
					goto failed;
				}
				if(source == mmx::addr_t()) {
					vnx::log_error() << "Missing source address argument: -s | --source";
					goto failed;
				}
				if(target == mmx::addr_t()) {
					vnx::log_error() << "Missing destination address argument: -t | --target";
					goto failed;
				}
				const auto txid = wallet.send_from(index, mojo, target, source, contract);
				std::cout << "Sent " << mojo / pow(10, decimals) << " " << (token ? token->symbol : "MMX") << " (" << mojo << ") to " << target << std::endl;
				std::cout << "Transaction ID: " << txid << std::endl;
			}
			else if(command == "mint")
			{
				const auto token = std::dynamic_pointer_cast<const mmx::contract::Token>(node.get_contract(contract));
				if(!token) {
					vnx::log_error() << "No such token: " << contract;
					goto failed;
				}
				const int64_t mojo = amount * pow(10, token->decimals);
				if(amount <= 0 || mojo <= 0) {
					vnx::log_error() << "Invalid amount: " << amount << " (-a | --amount)";
					goto failed;
				}
				if(target == mmx::addr_t()) {
					vnx::log_error() << "Missing destination address argument: -t | --target";
					goto failed;
				}
				const auto txid = wallet.mint(index, mojo, target, contract);
				std::cout << "Minted " << mojo / pow(10, token->decimals) << " (" << mojo << ") " << token->symbol << " to " << target << std::endl;
				std::cout << "Transaction ID: " << txid << std::endl;
			}
			else if(command == "deploy")
			{
				std::string file_path;
				vnx::read_config("$3", file_path);

				auto payload = vnx::read_from_file<mmx::Contract>(file_path);
				if(!payload) {
					vnx::log_error() << "Failed to read contract from file: " << file_path;
					goto failed;
				}
				const auto txid = wallet.deploy(index, payload);
				std::cout << "Deployed " << payload->get_type_name() << " as " << mmx::addr_t(txid) << std::endl;
				std::cout << "Transaction ID: " << txid << std::endl;
			}
			else if(command == "log")
			{
				int64_t since = 0;
				vnx::read_config("$3", since);

				show_history(wallet.get_history(index, since), node, params);
			}
			else if(command == "create")
			{
				if(file_name.empty()) {
					file_name = "wallet.dat";
				}
				if(vnx::File(file_name).exists()) {
					vnx::log_error() << "Wallet file '" << file_name << "' already exists";
					goto failed;
				}
				std::string seed_str;
				vnx::read_config("$3", seed_str);

				mmx::KeyFile wallet;
				if(seed_str.empty()) {
					wallet.seed_value = mmx::hash_t::random();
				} else {
					if(seed_str.size() == 64) {
						wallet.seed_value.from_string(seed_str);
					} else {
						vnx::log_error() << "Invalid seed value: '" << seed_str << "'";
						goto failed;
					}
				}
				vnx::write_to_file(file_name, wallet);

				std::cout << "Created wallet '" << file_name << "' with seed: "
						<< std::endl << wallet.seed_value << std::endl;
			}
			else {
				std::cerr << "Help: mmx wallet [show | log | send | send_from | mint | deploy | create]" << std::endl;
			}
		}
		else if(module == "node")
		{
			std::string node_url = ":11331";
			vnx::read_config("node", node_url);

			vnx::Handle<vnx::Proxy> proxy = new vnx::Proxy("Proxy", vnx::Endpoint::from_url(node_url));
			proxy->forward_list = {"Router", "Node"};
			proxy.start_detached();
			try {
				params = node.get_params();
			} catch(...) {
				// ignore
			}
			mmx::RouterClient router("Router");

			if(command == "balance")
			{
				mmx::addr_t address;
				if(!vnx::read_config("$3", address)) {
					vnx::log_error() << "Missing address argument! (node balances <address>)";
					goto failed;
				}
				for(const auto& entry : node.get_total_balances({address}))
				{
					const auto contract = node.get_contract(entry.first);
					if(!std::dynamic_pointer_cast<const mmx::contract::NFT>(contract)) {
						const auto token = std::dynamic_pointer_cast<const mmx::contract::Token>(contract);
						const auto decimals = token ? token->decimals : params->decimals;
						std::cout << "Balance: " << entry.second / pow(10, decimals) << " " << (token ? token->symbol : "MMX") << " (" << entry.second << ")" << std::endl;
					}
				}
			}
			else if(command == "info")
			{
				const auto height = node.get_height();
				const auto peak = node.get_header_at(height);
				std::cout << "Synced: " << (node.get_synced_height() ? "Yes" : "No") << std::endl;
				std::cout << "Height: " << height << std::endl;
				std::cout << "Netspace: " << mmx::calc_total_netspace(params, peak->space_diff) / pow(1024, 4) << " TiB" << std::endl;
				for(uint32_t i = 0; i < 2 * params->finality_delay && i < height; ++i) {
					const auto hash = node.get_block_hash(height - i);
					std::cout << "Block[" << (height - i) << "] " << (hash ? *hash : mmx::hash_t()) << std::endl;
				}
			}
			else if(command == "history")
			{
				mmx::addr_t address;
				if(!vnx::read_config("$3", address)) {
					vnx::log_error() << "Missing address argument! (node history <address> [since])";
					goto failed;
				}
				int64_t since = 0;
				vnx::read_config("$4", since);

				show_history(node.get_history_for({address}, since), node, params);
			}
			else if(command == "peers")
			{
				auto info = router.get_peer_info();
				uint32_t max_height = 0;
				for(const auto& peer : info->peers) {
					max_height = std::max(max_height, peer.height);
				}
				for(const auto& peer : info->peers) {
					std::cout << "[" << peer.address << "]";
					for(size_t i = peer.address.size(); i < 15; ++i) {
						std::cout << " ";
					}
					std::cout << " height = ";
					const auto height = std::to_string(peer.height);
					for(size_t i = height.size(); i < std::to_string(max_height).size(); ++i) {
						std::cout << " ";
					}
					if(peer.is_synced) {
						std::cout << " ";
					} else {
						std::cout << "!";
					}
					std::cout << height;
					std::cout << ", " << vnx::to_string_value(peer.type) << " (" << peer.version / 100 << "." << peer.version % 100 << ")";
					const auto recv = (peer.bytes_recv * 1000) / peer.connect_time_ms;
					const auto send = (peer.bytes_send * 1000) / peer.connect_time_ms;
					std::cout << ", " << recv / 1024 << "." << (recv * 10 / 1024) % 10 << " KB/s recv";
					std::cout << ", " << send / 1024 << "." << (send * 10 / 1024) % 10 << " KB/s send";
					std::cout << ", since " << (peer.connect_time_ms / 60000) << " min";
					std::cout << ", " << peer.credits << " credits";
					std::cout << ", " << peer.tx_credits << " tx credits";
					std::cout << ", " << peer.ping_ms << " ms ping";
					std::cout << ", " << (peer.recv_timeout_ms / 100) / 10. << " sec timeout";
					if(peer.is_outbound) {
						std::cout << ", outbound";
					}
					if(peer.is_blocked) {
						std::cout << ", blocked";
					}
					std::cout << std::endl;
				}
			}
			else if(command == "sync")
			{
				node.start_sync(true);
				std::cout << "Started sync ..." << std::endl;
				while(true) {
					if(auto height = node.get_synced_height()) {
						std::cout << "Finished sync at height: " << *height << std::endl;
						break;
					}
					std::cout << node.get_height() << std::endl;
					std::this_thread::sleep_for(std::chrono::milliseconds(2000));
				}
			}
			else if(command == "discover")
			{
				router.discover();
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));

				const auto peers = router.get_known_peers();
				std::cout << "Got " << peers.size() << " known peers" << std::endl;
			}
			else if(command == "tx")
			{
				mmx::hash_t txid;
				vnx::read_config("$3", txid);

				const auto tx = node.get_transaction(txid, true);
				if(!tx) {
					vnx::log_error() << "No such transaction: " << txid;
					goto failed;
				}
				const auto height = node.get_height();
				const auto tx_height = node.get_tx_height(txid);
				if(tx_height) {
					std::cout << "Confirmations: " << height - (*tx_height) + 1 << std::endl;
				} else {
					std::cout << "Confirmations: none yet" << std::endl;
				}
				size_t i = 0;
				for(const auto& out : tx->outputs) {
					const auto token = std::dynamic_pointer_cast<const mmx::contract::Token>(node.get_contract(out.contract));
					const auto decimals = token ? token->decimals : params->decimals;
					std::cout << "Output[" << i++ << "]: " << out.amount / pow(10, decimals)
							<< " " << (token ? token->symbol : "MMX") << " (" << out.amount << ") -> " << out.address << std::endl;
				}
			}
			else if(command == "get")
			{
				std::string subject;
				vnx::read_config("$3", subject);

				if(subject == "balance")
				{
					mmx::addr_t address;
					vnx::read_config("$4", address);

					const auto token = std::dynamic_pointer_cast<const mmx::contract::Token>(node.get_contract(contract));
					std::cout << node.get_balance(address, contract) / pow(10, token ? token->decimals : params->decimals) << std::endl;
				}
				else if(subject == "amount")
				{
					mmx::addr_t address;
					vnx::read_config("$4", address);

					std::cout << node.get_balance(address, contract) << std::endl;
				}
				else if(subject == "height")
				{
					std::cout << node.get_height() << std::endl;
				}
				else if(subject == "block")
				{
					int64_t height = 0;
					vnx::read_config("$4", height);

					const auto block = node.get_block_at(height);
					{
						std::stringstream ss;
						vnx::PrettyPrinter printer(ss);
						vnx::accept(printer, block);
						std::cout << ss.str() << std::endl;
					}
				}
				else if(subject == "header")
				{
					int64_t height = 0;
					vnx::read_config("$4", height);

					const auto header = node.get_header_at(height);
					{
						std::stringstream ss;
						vnx::PrettyPrinter printer(ss);
						vnx::accept(printer, header);
						std::cout << ss.str() << std::endl;
					}
				}
				else if(subject == "tx")
				{
					mmx::hash_t txid;
					vnx::read_config("$4", txid);

					const auto tx = node.get_transaction(txid, true);
					{
						std::stringstream ss;
						vnx::PrettyPrinter printer(ss);
						vnx::accept(printer, tx);
						std::cout << ss.str() << std::endl;
					}
				}
				else if(subject == "contract")
				{
					mmx::addr_t address;
					vnx::read_config("$4", address);

					const auto contract = node.get_contract(address);
					{
						std::stringstream ss;
						vnx::PrettyPrinter printer(ss);
						vnx::accept(printer, contract);
						std::cout << ss.str() << std::endl;
					}
				}
				else if(subject == "peers")
				{
					int64_t max_count = 10;
					vnx::read_config("$4", max_count);

					for(const auto& peer : router.get_connected_peers()) {
						std::cout << peer << std::endl;
					}
				}
				else if(subject == "netspace")
				{
					const auto height = node.get_height();
					const auto peak = node.get_header_at(height);
					std::cout << mmx::calc_total_netspace(params, peak->space_diff) << std::endl;
				}
				else if(subject == "supply")
				{
					std::cout << node.get_total_supply(contract) << std::endl;
				}
				else {
					std::cerr << "Help: mmx node get [height | tx | contract | balance | amount | block | header | peers | netspace | supply]" << std::endl;
				}
			}
			else if(command == "fetch")
			{
				std::string subject;
				vnx::read_config("$3", subject);

				if(subject == "block" || subject == "header")
				{
					int64_t height = 0;
					std::string from_peer;
					vnx::read_config("$4", from_peer);
					vnx::read_config("$5", height);

					if(from_peer.empty()) {
						vnx::log_error() << "Missing peer argument: node fetch <peer> [height]";
						goto failed;
					}
					if(height < 0) {
						vnx::log_error() << "Invalid height: " << height;
						goto failed;
					}
					const auto block = router.fetch_block_at(from_peer, height);
					{
						std::stringstream ss;
						vnx::PrettyPrinter printer(ss);
						if(subject == "header") {
							vnx::accept(printer, block ? block->get_header() : nullptr);
						} else {
							vnx::accept(printer, block);
						}
						std::cout << ss.str() << std::endl;
					}
				}
				else {
					std::cerr << "Help: mmx node fetch [block | header]" << std::endl;
				}
			}
			else {
				std::cerr << "Help: mmx node [info | peers | tx | get | fetch | balance | history | sync]" << std::endl;
			}
		}
		else if(module == "farm")
		{
			std::string node_url = ":11333";
			vnx::read_config("node", node_url);

			vnx::Handle<vnx::Proxy> proxy = new vnx::Proxy("Proxy", vnx::Endpoint::from_url(node_url));
			proxy->forward_list = {"Farmer", "Harvester", "Node"};
			proxy.start_detached();

			mmx::FarmerClient farmer("Farmer");
			mmx::HarvesterClient harvester("Harvester");

			std::shared_ptr<const mmx::FarmInfo> info;
			try {
				info = farmer.get_farm_info();
			} catch(...) {
				// ignore
			}
			if(!info) {
				info = harvester.get_farm_info();
			}

			if(command == "info") {
				std::cout << "Total space: " << info->total_bytes / pow(1024, 4) << " TiB" << std::endl;
				for(const auto& entry : info->plot_count) {
					std::cout << "K" << int(entry.first) << ": " << entry.second << " plots" << std::endl;
				}
			}
			else if(command == "reload") {
				harvester.reload();
			}
			else if(command == "get") {
				std::string subject;
				vnx::read_config("$3", subject);

				if(subject == "space")
				{
					std::cout << info->total_bytes << std::endl;
				}
				else if(subject == "dirs")
				{
					for(const auto& entry : info->plot_dirs) {
						std::cout << entry << std::endl;
					}
				}
				else {
					std::cerr << "Help: mmx farm get [space | dirs]" << std::endl;
				}
			}
			else {
				std::cerr << "Help: mmx farm [info | get | reload]" << std::endl;
			}
		}
		else {
			std::cerr << "Help: mmx [node | wallet | farm]" << std::endl;
		}
	}
	catch(std::exception& ex) {
		vnx::log_error() << "Failed with: " << ex.what();
		goto failed;
	}

	goto exit;
failed:
	did_fail = true;
exit:
	vnx::close();

	mmx::secp256k1_free();

	return did_fail ? -1 : 0;
}



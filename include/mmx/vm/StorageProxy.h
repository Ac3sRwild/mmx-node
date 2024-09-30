/*
 * StorageProxy.h
 *
 *  Created on: Apr 24, 2022
 *      Author: mad
 */

#ifndef INCLUDE_MMX_VM_STORAGEPROXY_H_
#define INCLUDE_MMX_VM_STORAGEPROXY_H_

#include <mmx/vm/Storage.h>


namespace mmx {
namespace vm {

class Engine;

class StorageProxy : public Storage {
public:
	Engine* const engine;
	const std::shared_ptr<Storage> backend;
	const bool read_only;

	StorageProxy(Engine* engine, std::shared_ptr<Storage> backend, bool read_only);

	std::unique_ptr<var_t> read(const addr_t& contract, const uint64_t src) const override;

	std::unique_ptr<var_t> read(const addr_t& contract, const uint64_t src, const uint64_t key) const override;

	void write(const addr_t& contract, const uint64_t dst, const var_t& value) override;

	void write(const addr_t& contract, const uint64_t dst, const uint64_t key, const var_t& value) override;

	uint64_t lookup(const addr_t& contract, const var_t& value) const override;

	std::unique_ptr<uint128> get_balance(const addr_t& contract, const addr_t& currency) const override;

	using Storage::write;
	using Storage::lookup;

private:
	std::unique_ptr<var_t> read_ex(std::unique_ptr<var_t> var) const;

};


} // vm
} // mmx



#endif /* INCLUDE_MMX_VM_STORAGEPROXY_H_ */

/*
 * Storage.h
 *
 *  Created on: Apr 22, 2022
 *      Author: mad
 */

#ifndef INCLUDE_MMX_VM_STORAGE_H_
#define INCLUDE_MMX_VM_STORAGE_H_

#include <mmx/addr_t.hpp>
#include <mmx/uint128.hpp>
#include <mmx/vm/var_t.h>
#include <mmx/vm/varptr_t.hpp>

#include <memory>


namespace mmx {
namespace vm {

class Storage {
public:
	virtual ~Storage() {}

	virtual std::unique_ptr<var_t> read(const addr_t& contract, const uint64_t src) const = 0;

	virtual std::unique_ptr<var_t> read(const addr_t& contract, const uint64_t src, const uint64_t key) const = 0;

	virtual void write(const addr_t& contract, const uint64_t dst, const var_t& value) = 0;

	virtual void write(const addr_t& contract, const uint64_t dst, const uint64_t key, const var_t& value) = 0;

	virtual uint64_t lookup(const addr_t& contract, const var_t& value) const = 0;


	void write(const addr_t& contract, const uint64_t dst, const varptr_t& value) {
		if(value) {
			write(contract, dst, *value);
		}
	}

	void write(const addr_t& contract, const uint64_t dst, const uint64_t key, const varptr_t& value) {
		if(value) {
			write(contract, dst, key, *value);
		}
	}

	uint64_t lookup(const addr_t& contract, const varptr_t& value) const {
		if(value) {
			return lookup(contract, *value);
		}
		return 0;
	}

	virtual void set_balance(const addr_t& contract, const addr_t& currency, const uint128& amount) {
		throw std::logic_error("not implemented");
	}

	virtual std::unique_ptr<uint128> get_balance(const addr_t& contract, const addr_t& currency) const {
		throw std::logic_error("not implemented");
	}

};


} // vm
} // mmx

#endif /* INCLUDE_MMX_VM_STORAGE_H_ */

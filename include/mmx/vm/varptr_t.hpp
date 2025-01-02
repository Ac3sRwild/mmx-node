/*
 * varptr_t.hpp
 *
 *  Created on: May 12, 2022
 *      Author: mad
 */

#ifndef INCLUDE_MMX_VM_VARPTR_T_HPP_
#define INCLUDE_MMX_VM_VARPTR_T_HPP_

#include <mmx/vm/var_t.h>

#include <vnx/Input.h>
#include <vnx/Output.h>
#include <vnx/Visitor.h>


namespace mmx {
namespace vm {

class varptr_t {
public:
	varptr_t() = default;

	varptr_t(const varptr_t&) = default;

	varptr_t(const std::nullptr_t&) {}

	template<typename T>
	varptr_t(std::unique_ptr<T> var) {
		ptr = std::move(var);
	}

	var_t* get() const {
		return ptr.get();
	}

	operator bool() const {
		return bool(ptr);
	}

	var_t& operator*() {
		return *ptr;
	}

	const var_t& operator*() const {
		return *ptr;
	}

	var_t* operator->() {
		return get();
	}

	const var_t* operator->() const {
		return get();
	}

private:
	std::shared_ptr<var_t> ptr;

};

inline bool operator<(const varptr_t& lhs, const varptr_t& rhs) {
	return compare(lhs.get(), rhs.get()) < 0;
}

inline bool operator>(const varptr_t& lhs, const varptr_t& rhs) {
	return compare(lhs.get(), rhs.get()) > 0;
}

inline bool operator==(const varptr_t& lhs, const varptr_t& rhs) {
	return compare(lhs.get(), rhs.get()) == 0;
}

inline bool operator!=(const varptr_t& lhs, const varptr_t& rhs) {
	return compare(lhs.get(), rhs.get()) != 0;
}

inline std::string to_string(const varptr_t& var) {
	return to_string(var.get());
}

inline std::string to_string_value(const varptr_t& var) {
	return to_string_value(var.get());
}

inline std::string to_string_value_hex(const varptr_t& var) {
	return to_string_value_hex(var.get());
}

inline uint64_t to_ref(const varptr_t& var) {
	return to_ref(var.get());
}

inline uint256_t to_uint(const varptr_t& var) {
	return to_uint(var.get());
}

inline hash_t to_hash(const varptr_t& var) {
	return to_hash(var.get());
}

inline addr_t to_addr(const varptr_t& var) {
	return to_addr(var.get());
}

inline uint32_t get_size(const varptr_t& var) {
	return get_size(var.get());
}


} // vm
} // mmx


namespace vnx {

void read(vnx::TypeInput& in, mmx::vm::varptr_t& value, const vnx::TypeCode* type_code, const uint16_t* code);
void write(vnx::TypeOutput& out, const mmx::vm::varptr_t& value, const vnx::TypeCode* type_code, const uint16_t* code);

void read(std::istream& in, mmx::vm::varptr_t& value);
void write(std::ostream& out, const mmx::vm::varptr_t& value);

void accept(vnx::Visitor& visitor, const mmx::vm::varptr_t& value);

} // vnx

#endif /* INCLUDE_MMX_VM_VARPTR_T_HPP_ */

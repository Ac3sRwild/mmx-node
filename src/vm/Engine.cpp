/*
 * Engine.cpp
 *
 *  Created on: Apr 22, 2022
 *      Author: mad
 */

#include <mmx/vm/Engine.h>
#include <mmx/vm/StorageProxy.h>
#include <mmx/vm_interface.h>
#include <mmx/signature_t.hpp>
#include <mmx/error_code_e.hxx>

#include <iostream>


namespace mmx {
namespace vm {

static const uint64_t MEM_HEAP_NEXT_ALLOC = MEM_HEAP + GLOBAL_NEXT_ALLOC;

Engine::Engine(const addr_t& contract, std::shared_ptr<Storage> backend, bool read_only)
	:	contract(contract),
		storage(std::make_shared<StorageProxy>(this, backend, read_only))
{
}

Engine::~Engine()
{
	key_map.clear();
}

void Engine::addref(const uint64_t dst)
{
	if(auto var = read(dst, true)) {
		var->addref();
	}
}

void Engine::unref(const uint64_t dst)
{
	if(auto var = read(dst, true)) {
		if(var->unref()) {
			erase(dst);
		}
	}
}

var_t* Engine::assign(const uint64_t dst, std::unique_ptr<var_t> value)
{
	if(have_init && dst < MEM_EXTERN) {
		throw std::logic_error("already initialized");
	}
	switch(value->type) {
		case TYPE_ARRAY:
			((array_t*)value.get())->address = dst;
			break;
		case TYPE_MAP:
			((map_t*)value.get())->address = dst;
			break;
		default:
			break;
	}
	auto& var = memory[dst];
	if(!var && dst >= MEM_STATIC && dst < new_heap_base) {
		var = storage->read(contract, dst);
	}
	return assign(var, std::move(value));
}

var_t* Engine::assign(std::unique_ptr<var_t>& var, std::unique_ptr<var_t> value)
{
	if(var) {
		if(var->flags & FLAG_CONST) {
			throw std::logic_error("read-only memory");
		}
		value->flags = var->flags;
		value->ref_count = var->ref_count;
		clear(var.get());
	}
	const auto size = num_bytes(value.get());
	if(size > MAX_VALUE_BYTES) {
		throw std::runtime_error("value size too large: " + std::to_string(size) + " bytes");
	}
	gas_used += WRITE_COST + (size * WRITE_32_BYTE_COST) / 32;

	var = std::move(value);
	var->flags |= FLAG_DIRTY;
	var->flags &= ~FLAG_DELETED;

	if(var->type == TYPE_REF) {
		addref(((const ref_t*)var.get())->address);
	}
	return var.get();
}

uint64_t Engine::lookup(const uint64_t src, const bool read_only)
{
	return lookup(read_fail(src), read_only);
}

uint64_t Engine::lookup(const var_t* var, const bool read_only)
{
	return var ? lookup(*var, read_only) : 0;
}

uint64_t Engine::lookup(const varptr_t& var, const bool read_only)
{
	return lookup(var.get(), read_only);
}

uint64_t Engine::lookup(const var_t& var, const bool read_only)
{
	if(var.type == TYPE_NIL) {
		throw std::runtime_error("invalid key type");
	}
	const auto size = num_bytes(var);
	if(size > MAX_KEY_BYTES) {
		throw std::runtime_error("invalid key size: " + std::to_string(size));
	}
	const auto iter = key_map.find(&var);
	if(iter != key_map.end()) {
		return iter->second;
	}
	if(auto key = storage->lookup(contract, var)) {
		const auto& value = read_fail(key);
		if(!(value.flags & FLAG_CONST) || !(value.flags & FLAG_KEY)) {
			throw std::logic_error("lookup(): key missing flag CONST / KEY");
		}
		key_map[&value] = key;
		return key;
	}
	if(read_only) {
		return 0;
	}
	const auto key = alloc();
	const auto value = write(key, var);
	value->flags |= FLAG_CONST | FLAG_KEY;
	value->addref();
	key_map[value] = key;
	return key;
}

var_t* Engine::write(const uint64_t dst, const var_t* src)
{
	if(src) {
		return write(dst, *src);
	}
	return write(dst, var_t());
}

var_t* Engine::write(const uint64_t dst, const var_t& src)
{
	if(have_init && dst < MEM_EXTERN) {
		throw std::logic_error("already initialized");
	}
	auto& var = memory[dst];
	if(!var && dst >= MEM_STATIC && dst < new_heap_base) {
		var = storage->read(contract, dst);
	}
	return write(var, &dst, src);
}

var_t* Engine::write(const uint64_t dst, const varptr_t& var)
{
	return write(dst, var.get());
}

var_t* Engine::write(std::unique_ptr<var_t>& var, const uint64_t* dst, const var_t& src)
{
	if(var.get() == &src) {
		return var.get();
	}
	if(src.flags & FLAG_DELETED) {
		throw std::logic_error("write(): src has FLAG_DELETED");
	}
	if(var) {
		if(var->flags & FLAG_CONST) {
			if(dst) {
				throw std::logic_error("read-only memory at " + to_hex(*dst));
			} else {
				throw std::logic_error("read-only memory");
			}
		}
		if(var->flags & FLAG_DELETED) {
			var->flags |= FLAG_DIRTY;
			var->flags &= ~FLAG_DELETED;
		}
		switch(src.type) {
			case TYPE_NIL:
			case TYPE_TRUE:
			case TYPE_FALSE:
				switch(var->type) {
					case TYPE_NIL:
					case TYPE_TRUE:
					case TYPE_FALSE:
						if(var->type != src.type) {
							var->flags |= FLAG_DIRTY;
							var->type = src.type;
						}
						return var.get();
					default:
						break;
				}
				break;
			case TYPE_REF:
				if(var->type == TYPE_REF) {
					auto ref = (ref_t*)var.get();
					const auto new_address = ((const ref_t&)src).address;
					if(new_address != ref->address) {
						unref(ref->address);
						var->flags |= FLAG_DIRTY;
						ref->address = new_address;
						addref(ref->address);
					}
					return ref;
				}
				break;
			case TYPE_UINT:
				if(var->type == TYPE_UINT) {
					auto uint = (uint_t*)var.get();
					const auto new_value = ((const uint_t&)src).value;
					if(new_value != uint->value) {
						var->flags |= FLAG_DIRTY;
						uint->value = new_value;
					}
					return uint;
				}
				break;
			default:
				break;
		}
	}
	switch(src.type) {
		case TYPE_NIL:
		case TYPE_TRUE:
		case TYPE_FALSE:
			assign(var, std::make_unique<var_t>(src.type));
			break;
		case TYPE_REF:
			assign(var, std::make_unique<ref_t>(((const ref_t&)src).address));
			break;
		case TYPE_UINT:
			assign(var, std::make_unique<uint_t>(((const uint_t&)src).value));
			break;
		case TYPE_STRING:
		case TYPE_BINARY:
			assign(var, binary_t::alloc((const binary_t&)src));
			break;
		case TYPE_ARRAY: {
			if(!dst) {
				throw std::logic_error("cannot assign array here");
			}
			const auto& asrc = (const array_t&)src;
			{
				auto tmp = std::make_unique<array_t>(asrc.size);
				tmp->address = *dst;
				assign(var, std::move(tmp));
			}
			for(uint64_t i = 0; i < asrc.size; ++i) {
				write_entry(*dst, i, read_entry_fail(asrc.address, i));
				check_gas();
			}
			break;
		}
		case TYPE_MAP: {
			if(!dst) {
				throw std::logic_error("cannot assign map here");
			}
			if(src.flags & FLAG_STORED) {
				throw std::logic_error("cannot clone map from storage");
			}
			const auto& msrc = (const map_t&)src;
			{
				auto tmp = std::make_unique<map_t>();
				tmp->address = *dst;
				assign(var, std::move(tmp));
			}
			const auto begin = entries.lower_bound(std::make_pair(msrc.address, 0));
			for(auto iter = begin; iter != entries.end() && iter->first.first == msrc.address; ++iter) {
				if(auto value = iter->second.get()) {
					if((value->flags & FLAG_DELETED) == 0) {
						write_entry(*dst, iter->first.second, *value);
						check_gas();
					}
				}
			}
			break;
		}
		default:
			throw invalid_type(src);
	}
	return var.get();
}

void Engine::push_back(const uint64_t dst, const var_t& var)
{
	auto& array = read_fail<array_t>(dst, TYPE_ARRAY);
	if(array.size >= std::numeric_limits<uint32_t>::max()) {
		throw std::runtime_error("push_back() overflow at " + to_hex(dst));
	}
	write_entry(dst, array.size++, var);
	array.flags |= FLAG_DIRTY;
}

void Engine::push_back(const uint64_t dst, const varptr_t& var)
{
	if(var) {
		push_back(dst, *var);
	} else {
		push_back(dst, var_t());
	}
}

void Engine::push_back(const uint64_t dst, const uint64_t src)
{
	if(dst == src) {
		throw std::logic_error("push_back(): dst == src");
	}
	push_back(dst, read_fail(src));
}

void Engine::pop_back(const uint64_t dst, const uint64_t& src)
{
	if(dst == src) {
		throw std::logic_error("pop_back(): dst == src");
	}
	auto& array = read_fail<array_t>(src, TYPE_ARRAY);
	if(array.size == 0) {
		throw std::logic_error("pop_back() underflow at " + to_hex(dst));
	}
	const auto index = array.size - 1;
	write(dst, read_entry_fail(src, index));
	erase_entry(src, index);
	array.size--;
	array.flags |= FLAG_DIRTY;
}

var_t* Engine::write_entry(const uint64_t dst, const uint64_t key, const var_t& src)
{
	if(have_init && dst < MEM_EXTERN) {
		throw std::logic_error("already initialized");
	}
	auto& container = read_fail(dst);

	if(container.flags & FLAG_CONST) {
		throw std::logic_error("read-only memory at " + to_hex(dst));
	}
	auto& var = entries[std::make_pair(dst, key)];

	// load potential previous value if container is stored
	// TODO: still needed?
	// TODO: because no more ref counting for stored values
	if(!var && dst >= MEM_STATIC && (container.flags & FLAG_STORED)) {
		var = storage->read(contract, dst, key);
	}
	return write(var, nullptr, src);
}

var_t* Engine::write_entry(const uint64_t dst, const uint64_t key, const varptr_t& var)
{
	if(var) {
		return write_entry(dst, key, *var);
	}
	return write_entry(dst, key, var_t());
}

var_t* Engine::write_key(const uint64_t dst, const uint64_t key, const var_t& var)
{
	return write_entry(dst, lookup(key, false), var);
}

var_t* Engine::write_key(const uint64_t dst, const var_t& key, const var_t& var)
{
	return write_entry(dst, lookup(key, false), var);
}

var_t* Engine::write_key(const uint64_t dst, const varptr_t& key, const varptr_t& var)
{
	return write_entry(dst, lookup(key, false), var);
}

void Engine::erase_entry(const uint64_t dst, const uint64_t key)
{
	const auto mapkey = std::make_pair(dst, key);
	auto iter = entries.find(mapkey);
	if(iter == entries.end() && dst >= MEM_STATIC && (read_fail(dst).flags & FLAG_STORED)) {
		if(auto var = storage->read(contract, dst, key)) {
			iter = entries.emplace(mapkey, std::move(var)).first;
		}
	}
	if(iter != entries.end()) {
		erase(iter->second);
	}
}

void Engine::erase_key(const uint64_t dst, const uint64_t key)
{
	if(auto addr = lookup(key, true)) {
		erase_entry(dst, addr);
	}
}

void Engine::erase_entries(const uint64_t dst)
{
	const auto begin = entries.lower_bound(std::make_pair(dst, 0));
	for(auto iter = begin; iter != entries.end() && iter->first.first == dst; ++iter) {
		erase(iter->second);
		check_gas();
	}
}

void Engine::erase(const uint64_t dst)
{
	const auto iter = memory.find(dst);
	if(iter != memory.end()) {
		erase(iter->second);
	}
	else if(dst >= MEM_STATIC) {
		if(auto var = storage->read(contract, dst)) {
			erase(memory[dst] = std::move(var));
		}
	}
}

void Engine::erase(std::unique_ptr<var_t>& var)
{
	if(!var) {
		return;
	}
	if(var->flags & FLAG_CONST) {
		throw std::logic_error("erase() on read-only memory");
	}
	if(var->flags & FLAG_DELETED) {
		return;
	}
	if(var->ref_count) {
		throw std::logic_error("erase() with ref_count " + std::to_string(var->ref_count));
	}
	if(++erase_call_depth > MAX_ERASE_RECURSION) {
		throw std::runtime_error("erase() recursion overflow");
	}
	// account cost here because of recursion in clear() -> unref() -> erase() -> clear()
	gas_used += WRITE_COST;
	check_gas();

	clear(var.get());

	const auto prev = std::move(var);
	var = std::make_unique<var_t>();
	if(prev->flags & FLAG_STORED) {
		var->flags = (prev->flags | FLAG_DIRTY | FLAG_DELETED);
	} else {
		var->flags = FLAG_DELETED;
	}
	erase_call_depth--;
}

void Engine::clear(var_t* var)
{
	switch(var->type) {
		case TYPE_REF:
			unref(((const ref_t*)var)->address);
			break;
		case TYPE_ARRAY: {
			if(var->flags & FLAG_STORED) {
				throw std::logic_error("cannot erase array in storage");
			}
			erase_entries(((const array_t*)var)->address);
			break;
		}
		case TYPE_MAP: {
			if(var->flags & FLAG_STORED) {
				throw std::logic_error("cannot erase map in storage");
			}
			erase_entries(((const map_t*)var)->address);
			break;
		}
		default:
			break;
	}
}

var_t* Engine::read(const uint64_t src, const bool mem_only)
{
	auto iter = memory.find(src);
	if(iter != memory.end()) {
		auto var = iter->second.get();
		if(var) {
			if(var->flags & FLAG_DELETED) {
				return nullptr;
			}
		}
		return var;
	}
	if(mem_only) {
		return nullptr;
	}
	if(src >= MEM_STATIC) {
		if(auto var = storage->read(contract, src)) {
			return (memory[src] = std::move(var)).get();
		}
	}
	return nullptr;
}

var_t& Engine::read_fail(const uint64_t src)
{
	if(auto var = read(src)) {
		return *var;
	}
	throw std::runtime_error("read fail at " + to_hex(src));
}

var_t* Engine::read_entry(const uint64_t src, const uint64_t key)
{
	const auto mapkey = std::make_pair(src, key);
	const auto iter = entries.find(mapkey);
	if(iter != entries.end()) {
		auto var = iter->second.get();
		if(var) {
			if(var->flags & FLAG_DELETED) {
				return nullptr;
			}
		}
		return var;
	}
	if(src >= MEM_STATIC) {
		if(auto var = storage->read(contract, src, key)) {
			return (entries[mapkey] = std::move(var)).get();
		}
	}
	return nullptr;
}

var_t& Engine::read_entry_fail(const uint64_t src, const uint64_t key)
{
	if(auto var = read_entry(src, key)) {
		return *var;
	}
	throw std::logic_error("read fail at " + to_hex(src) + "[" + std::to_string(key) + "]");
}

var_t* Engine::read_key(const uint64_t src, const uint64_t key)
{
	if(auto addr = lookup(key, true)) {
		return read_entry(src, addr);
	}
	return nullptr;
}

var_t* Engine::read_key(const uint64_t src, const var_t& key)
{
	if(auto addr = lookup(key, true)) {
		return read_entry(src, addr);
	}
	return nullptr;
}

var_t* Engine::read_key(const uint64_t src, const varptr_t& key)
{
	if(auto addr = lookup(key, true)) {
		return read_entry(src, addr);
	}
	return nullptr;
}

var_t& Engine::read_key_fail(const uint64_t src, const uint64_t key)
{
	if(auto addr = lookup(key, true)) {
		return read_entry_fail(src, addr);
	}
	throw std::logic_error("no such key at " + to_hex(src));
}

uint64_t Engine::alloc()
{
	auto offset = read<uint_t>(MEM_HEAP_NEXT_ALLOC, TYPE_UINT);
	if(!offset) {
		new_heap_base = MEM_HEAP;
		offset = (uint_t*)assign(MEM_HEAP_NEXT_ALLOC, std::make_unique<uint_t>(MEM_HEAP + GLOBAL_DYNAMIC_START));
		offset->pin();
	}
	if(offset->value >= uint64_t(-1)) {
		throw std::runtime_error("out of memory");
	}
	offset->flags |= FLAG_DIRTY;

	if(first_alloc) {
		first_alloc = false;
		new_heap_base = offset->value;
	}
	return offset->value++;
}

void Engine::init()
{
	if(have_init) {
		throw std::logic_error("init(): already initialized");
	}
	// Note: address 0 is not a valid key (used to denote "key not found")
	for(auto iter = memory.lower_bound(1); iter != memory.lower_bound(MEM_EXTERN); ++iter) {
		const auto* key = iter->second.get();
		if(num_bytes(key) <= MAX_KEY_BYTES) {
			key_map.emplace(key, iter->first);
		}
	}
	have_init = true;
}

void Engine::begin(const uint64_t instr_ptr)
{
	if(!have_init) {
		throw std::logic_error("begin(): not initialized");
	}
	if(!call_stack.empty()) {
		throw std::logic_error("begin(): call stack not empty");
	}
	for(auto iter = memory.begin(); iter != memory.lower_bound(MEM_STACK); ++iter) {
		if(auto var = iter->second.get()) {
			var->flags |= FLAG_CONST;
			var->flags &= ~FLAG_DIRTY;
		}
	}
	frame_t frame;
	frame.instr_ptr = instr_ptr;
	call_stack.push_back(frame);
}

void Engine::run()
{
	while(!call_stack.empty()) {
		step();
	}
}

void Engine::step()
{
	const auto instr_ptr = get_frame().instr_ptr;
	if(instr_ptr >= code.size()) {
		throw std::logic_error("instr_ptr out of bounds: " + to_hex(instr_ptr) + " > " + to_hex(code.size()));
	}
	try {
		exec(code[instr_ptr]);
		check_gas();
	} catch(...) {
		if(is_debug) {
			dump_memory();
		}
		error_addr = instr_ptr;
		throw;
	}
}

void Engine::check_gas()
{
	if(gas_used > gas_limit) {
		error_code = error_code_e::TXFEE_OVERRUN;
		throw std::runtime_error("out of gas: " + std::to_string(gas_used) + " > " + std::to_string(gas_limit));
	}
}

void Engine::copy(const uint64_t dst, const uint64_t src)
{
	if(dst != src) {
		write(dst, read_fail(src));
	}
}

void Engine::clone(const uint64_t dst, const uint64_t src)
{
	const auto addr = alloc();
	write(addr, read_fail(src));
	write(dst, ref_t(addr));
}

void Engine::get(const uint64_t dst, const uint64_t addr, const uint64_t key, const uint8_t flags)
{
	const auto& var = read_fail(addr);
	switch(var.type) {
		case TYPE_ARRAY: {
			const auto& index = read_fail<uint_t>(key, TYPE_UINT);
			if(index.value < ((const array_t&)var).size) {
				write(dst, read_entry_fail(addr, uint64_t(index.value)));
			} else if(flags & OPFLAG_HARD_FAIL) {
				throw std::runtime_error("array index out of bounds");
			} else {
				write(dst, var_t());
			}
			break;
		}
		case TYPE_MAP:
			write(dst, read_key(addr, key));
			break;
		default:
			if(flags & OPFLAG_HARD_FAIL) {
				throw invalid_type(var);
			} else {
				write(dst, var_t());
			}
	}
}

void Engine::set(const uint64_t addr, const uint64_t key, const uint64_t src, const uint8_t flags)
{
	const auto& var = read_fail(addr);
	switch(var.type) {
		case TYPE_ARRAY: {
			const auto& index = read_fail<uint_t>(key, TYPE_UINT);
			if(index.value >= ((const array_t&)var).size) {
				throw std::runtime_error("array index out of bounds");
			}
			write_entry(addr, uint64_t(index.value), read_fail(src));
			break;
		}
		case TYPE_MAP:
			write_key(addr, key, read_fail(src));
			break;
		default:
			throw invalid_type(var);
	}
}

void Engine::erase(const uint64_t addr, const uint64_t key, const uint8_t flags)
{
	const auto& var = read_fail(addr);
	switch(var.type) {
		case TYPE_MAP:
			erase_key(addr, key);
			break;
		default:
			throw invalid_type(var);
	}
}

void Engine::concat(const uint64_t dst, const uint64_t lhs, const uint64_t rhs)
{
	const auto& lvar = read_fail(lhs);
	const auto& rvar = read_fail(rhs);
	if(lvar.type != rvar.type) {
		throw std::logic_error("type mismatch");
	}
	switch(lvar.type) {
		case TYPE_STRING:
		case TYPE_BINARY: {
			const auto& L = (const binary_t&)lvar;
			const auto& R = (const binary_t&)rvar;
			const auto size = uint64_t(L.size) + R.size;
			auto res = binary_t::unsafe_alloc(size, lvar.type);
			::memcpy(res->data(), L.data(), L.size);
			::memcpy(res->data(L.size), R.data(), R.size);
			res->size = size;
			assign(dst, std::move(res));
			break;
		}
		case TYPE_ARRAY: {
			if(dst == lhs || dst == rhs) {
				throw std::logic_error("dst == lhs || dst == rhs");
			}
			const auto addr = alloc();
			const auto& L = (const array_t&)lvar;
			const auto& R = (const array_t&)rvar;
			assign(addr, std::make_unique<array_t>());
			for(uint64_t i = 0; i < L.size; ++i) {
				push_back(addr, read_entry_fail(L.address, i));
				check_gas();
			}
			for(uint64_t i = 0; i < R.size; ++i) {
				push_back(addr, read_entry_fail(R.address, i));
				check_gas();
			}
			write(dst, ref_t(addr));
			break;
		}
		default:
			throw invalid_type(lvar);
	}
}

void Engine::memcpy(const uint64_t dst, const uint64_t src, const uint64_t count, const uint64_t offset)
{
	const auto& svar = read_fail(src);
	switch(svar.type) {
		case TYPE_STRING:
		case TYPE_BINARY: {
			const auto& sbin = (const binary_t&)svar;
			if(sbin.size < offset + count) {
				throw std::logic_error("out of bounds read");
			}
			auto res = binary_t::unsafe_alloc(count, svar.type);
			::memcpy(res->data(), sbin.data(offset), count);
			res->size = count;
			assign(dst, std::move(res));
			break;
		}
		default:
			throw invalid_type(svar);
	}
}

void Engine::sha256(const uint64_t dst, const uint64_t src)
{
	const auto& svar = read_fail(src);
	switch(svar.type) {
		case TYPE_STRING:
		case TYPE_BINARY: {
			const auto& sbin = (const binary_t&)svar;
			gas_used += ((sbin.size + 63) / 64) * SHA256_BLOCK_COST;
			check_gas();
			const hash_t hash(sbin.data(), sbin.size);
			write(dst, binary_t::alloc(hash.data(), hash.size()));
			break;
		}
		default:
			throw invalid_type(svar);
	}
}

void Engine::verify(const uint64_t dst, const uint64_t msg, const uint64_t pubkey, const uint64_t signature)
{
	gas_used += ECDSA_VERIFY_COST;
	check_gas();
	write(dst, var_t(
			signature_t(read_fail<binary_t>(signature, TYPE_BINARY).to_vector()).verify(
					pubkey_t(read_fail<binary_t>(pubkey, TYPE_BINARY).to_vector()),
					hash_t::from_bytes(read_fail<binary_t>(msg, TYPE_BINARY).to_vector()))));
}

void Engine::conv(const uint64_t dst, const uint64_t src, const uint64_t dflags, const uint64_t sflags)
{
	const uint8_t dflags_0 = dflags;
	const uint8_t dflags_1 = (dflags >> 8);
	const uint8_t sflags_0 = sflags;
//	const uint8_t sflags_1 = (sflags >> 8);

	const auto& svar = read_fail(src);
	switch(svar.type) {
		case TYPE_NIL:
			switch(dflags_0) {
				case CONVTYPE_BOOL: write(dst, var_t(TYPE_FALSE)); break;
				case CONVTYPE_UINT: write(dst, uint_t()); break;
				case CONVTYPE_ADDRESS: write(dst, vm::to_binary(addr_t())); break;
				default: throw std::logic_error("invalid conversion: NIL to " + to_hex(dflags));
			}
			break;
		case TYPE_TRUE:
			switch(dflags_0) {
				case CONVTYPE_BOOL: write(dst, var_t(TYPE_TRUE)); break;
				case CONVTYPE_UINT: write(dst, uint_t(1)); break;
				default: throw std::logic_error("invalid conversion: TRUE to " + to_hex(dflags));
			}
			break;
		case TYPE_FALSE:
			switch(dflags_0) {
				case CONVTYPE_BOOL: write(dst, var_t(TYPE_FALSE)); break;
				case CONVTYPE_UINT: write(dst, uint_t()); break;
				default: throw std::logic_error("invalid conversion: FALSE to " + to_hex(dflags));
			}
			break;
		case TYPE_UINT: {
			const auto& value = ((const uint_t&)svar).value;
			switch(dflags_0) {
				case CONVTYPE_BOOL:
					write(dst, var_t(value != uint256_0));
					break;
				case CONVTYPE_UINT:
					write(dst, svar);
					break;
				case CONVTYPE_STRING: {
					int base = 10;
					switch(dflags_1) {
						case CONVTYPE_DEFAULT: break;
						case CONVTYPE_BASE_2: base = 2; break;
						case CONVTYPE_BASE_8: base = 8; break;
						case CONVTYPE_BASE_10: base = 10; break;
						case CONVTYPE_BASE_16: base = 16; break;
						default: throw std::logic_error("invalid conversion: UINT to STRING with dst flags " + to_hex(dflags));
					}
					if(value >> 128) {
						switch(base) {
							case 2:
							case 8:
							case 16: break;
							default: throw std::logic_error("invalid conversion: 256-bit UINT to STRING with base " + std::to_string(base));
						}
						gas_used += CONV_UINT_256_STRING_COST;
					} else if(value >> 64) {
						gas_used += CONV_UINT_128_STRING_COST;
					} else if(value >> 32) {
						gas_used += CONV_UINT_64_STRING_COST;
					} else {
						gas_used += CONV_UINT_32_STRING_COST;
					}
					check_gas();
					assign(dst, binary_t::alloc(value.str(base)));
					break;
				}
				case CONVTYPE_BINARY: {
					bool big_endian = true;
					switch(dflags_1) {
						case CONVTYPE_DEFAULT:
						case CONVTYPE_BIG_ENDIAN: break;
						case CONVTYPE_LITTLE_ENDIAN: big_endian = false; break;
						default: throw std::logic_error("invalid conversion: UINT to BINARY with dst flags " + to_hex(dflags));
					}
					bytes_t<32> tmp;
					tmp.from_uint(value, big_endian);
					assign(dst, binary_t::alloc(tmp.data(), tmp.size()));
					break;
				}
				default:
					throw std::logic_error("invalid conversion: UINT to " + to_hex(dflags));
			}
			break;
		}
		case TYPE_STRING: {
			const auto& sstr = (const binary_t&)svar;
			switch(dflags_0) {
				case CONVTYPE_BOOL:
					write(dst, var_t(bool(sstr.size)));
					break;
				case CONVTYPE_UINT: {
					auto value = sstr.to_string();
					const auto prefix_2 = value.substr(0, 2);

					int base = 10;
					switch(sflags_0) {
						case CONVTYPE_DEFAULT:
							if(prefix_2 == "0x") {
								base = 16;
							} else if(prefix_2 == "0b") {
								base = 2;
							}
							break;
						case CONVTYPE_BASE_2: base = 2; break;
						case CONVTYPE_BASE_8: base = 8; break;
						case CONVTYPE_BASE_10: base = 10; break;
						case CONVTYPE_BASE_16: base = 16; break;
						default: throw std::logic_error("invalid conversion: STRING to UINT with src flags " + to_hex(sflags));
					}
					if(prefix_2 == "0x" || prefix_2 == "0b") {
						value = value.substr(2);
					}
					for(const auto c : value) {
						int digit = -1;
						if('0' <= c && c <= '9') {
							digit = c - '0';
						} else if('a' <= c && c <= 'z') {
							digit = c - 'a' + 10;
						} else if('A' <= c && c <= 'Z') {
							digit = c - 'A' + 10;
						}
						if(digit < 0 || digit >= base) {
							throw std::logic_error("invalid string of base " + std::to_string(base) + ": " + value);
						}
					}
					gas_used += value.size() * CONV_STRING_UINT_CHAR_COST;
					check_gas();
					write(dst, uint_t(uint256_t(value.c_str(), base)));
					break;
				}
				case CONVTYPE_STRING:
					write(dst, svar);
					break;
				case CONVTYPE_BINARY: {
					int base = 0;
					switch(sflags_0) {
						case CONVTYPE_DEFAULT: break;
						case CONVTYPE_BASE_16: base = 16; break;
						default: throw std::logic_error("invalid conversion: STRING to BINARY with src flags " + to_hex(sflags));
					}
					switch(base) {
						case 0:
							assign(dst, binary_t::alloc(sstr, TYPE_BINARY));
							break;
						case 16: {
							const auto value = sstr.to_string();
							if(value.size() % 2) {
								throw std::runtime_error("invalid conversion: STRING to BINARY with source length " + std::to_string(value.size()) + " % 2 != 0");
							}
							const auto prefix = (value.substr(0, 2) == "0x" ? 2 : 0);
							for(size_t i = prefix; i < value.size(); ++i) {
								const auto c = value[i];
								if(!('0' <= c && c <= '9') && !('a' <= c && c <= 'f') && !('A' <= c && c <= 'F')) {
									throw std::runtime_error("invalid hex string: " + value);
								}
							}
							const auto tmp = vnx::from_hex_string(value);
							assign(dst, binary_t::alloc(tmp.data(), tmp.size(), TYPE_BINARY));
							break;
						}
						default:
							throw std::logic_error("invalid conversion: STRING to BINARY with base " + std::to_string(base));
					}
					break;
				}
				case CONVTYPE_ADDRESS: {
					const auto value = sstr.to_string();
					gas_used += CONV_STRING_BECH32_COST;
					check_gas();
					const addr_t tmp(value);
					assign(dst, binary_t::alloc(tmp.data(), tmp.size()));
					break;
				}
				default:
					throw std::logic_error("invalid conversion: STRING to " + to_hex(dflags));
			}
			break;
		}
		case TYPE_BINARY: {
			const auto& sbin = (const binary_t&)svar;
			switch(dflags_0) {
				case CONVTYPE_BOOL:
					write(dst, var_t(bool(sbin.size)));
					break;
				case CONVTYPE_UINT: {
					if(sbin.size != 32) {
						throw std::runtime_error("invalid conversion: BINARY to UINT: size != 32");
					}
					bool big_endian = true;
					switch(sflags_0) {
						case CONVTYPE_DEFAULT:
						case CONVTYPE_BIG_ENDIAN: break;
						case CONVTYPE_LITTLE_ENDIAN: big_endian = false; break;
						default: throw std::logic_error("invalid conversion: BINARY to UINT with src flags " + to_hex(sflags));
					}
					bytes_t<32> tmp;
					::memcpy(tmp.data(), sbin.data(), sbin.size);
					write(dst, uint_t(tmp.to_uint<uint256_t>(big_endian)));
					break;
				}
				case CONVTYPE_STRING: {
					int base = 0;
					switch(dflags_1) {
						case CONVTYPE_DEFAULT: break;
						case CONVTYPE_BASE_16: base = 16; break;
						case CONVTYPE_ADDRESS: base = 32; break;
						default: throw std::logic_error("invalid conversion: BINARY to STRING with dst flags " + to_hex(dflags));
					}
					switch(base) {
						case 32:
							gas_used += CONV_BECH32_STRING_COST;
							check_gas();
							assign(dst, binary_t::alloc(sbin.to_addr().to_string()));
							break;
						case 16:
							assign(dst, binary_t::alloc(sbin.to_hex_string()));
							break;
						case 0:
							assign(dst, binary_t::alloc(sbin.data(), sbin.size, TYPE_STRING));
							break;
						default:
							throw std::logic_error("invalid conversion: BINARY to STRING with base " + std::to_string(base));
					}
					break;
				}
				case CONVTYPE_BINARY:
					write(dst, svar);
					break;
				case CONVTYPE_ADDRESS:
					if(sbin.size != 32) {
						throw std::runtime_error("invalid conversion: BINARY to ADDRESS with source length " + std::to_string(sbin.size) + " != 32");
					}
					write(dst, svar);
					break;
				default:
					throw std::logic_error("invalid conversion: BINARY to " + to_hex(dflags));
			}
			break;
		}
		default:
			switch(dflags_0) {
				case CONVTYPE_BOOL:
					write(dst, var_t(TYPE_TRUE));
					break;
				default:
					throw std::logic_error("invalid conversion: " + to_hex(uint32_t(svar.type)) + " to " + to_hex(dflags));
			}
	}
}

void Engine::log(const uint64_t level, const uint64_t msg)
{
	if(log_func) {
		log_func(level, read_fail<binary_t>(msg, TYPE_STRING).to_string());
	}
}

void Engine::event(const uint64_t name, const uint64_t data)
{
	if(event_func) {
		event_func(read_fail<binary_t>(name, TYPE_STRING).to_string(), data);
	}
}

vnx::optional<std::string> Engine::parse_memo(const uint64_t addr)
{
	const auto& value = read_fail(addr);
	switch(value.type) {
		case TYPE_NIL:
			return nullptr;
		case TYPE_STRING: {
			const auto memo = ((const binary_t&)value).to_string();
			if(memo.size() > txio_t::MAX_MEMO_SIZE) {
				throw std::runtime_error("memo too large: " + memo);
			}
			if(memo.size()) {
				return memo;
			}
			return nullptr;
		}
		default:
			throw std::runtime_error("invalid memo type: " + to_string(value.type));
	}
	return nullptr;
}

void Engine::send(const uint64_t address, const uint64_t amount, const uint64_t currency, const uint64_t memo)
{
	const auto value = read_fail<uint_t>(amount, TYPE_UINT).value;
	if(value == 0) {
		return;
	}
	if(value >> 64) {
		throw std::runtime_error("send(): amount too large: 0x" + value.str(16));
	}
	if(currency == 0) {
		throw std::runtime_error("send(): currency == null");
	}
	auto& balance = get_balance(currency);
	if(balance < value) {
		throw std::runtime_error("send(): insufficient funds");
	}
	balance -= value;

	txout_t out;
	out.contract = read_fail<binary_t>(currency, TYPE_BINARY).to_addr();
	out.address = read_fail<binary_t>(address, TYPE_BINARY).to_addr();
	out.amount = value;
	out.memo = parse_memo(memo);
	outputs.push_back(out);
}

void Engine::mint(const uint64_t address, const uint64_t amount, const uint64_t memo)
{
	const auto value = read_fail<uint_t>(amount, TYPE_UINT).value;
	if(value == 0) {
		return;
	}
	if(value >> 64) {
		throw std::runtime_error("mint(): amount too large: 0x" + value.str(16));
	}
	txout_t out;
	out.contract = contract;
	out.address = read_fail<binary_t>(address, TYPE_BINARY).to_addr();
	out.amount = value;
	out.memo = parse_memo(memo);
	mint_outputs.push_back(out);
}

void Engine::rcall(const uint64_t name, const uint64_t method, const uint64_t stack_ptr, const uint64_t nargs)
{
	if(stack_ptr >= STACK_SIZE) {
		throw std::logic_error("stack overflow");
	}
	if(call_stack.size() >= MAX_CALL_RECURSION) {
		throw std::logic_error("recursion overflow");
	}
	if(nargs > 4096) {
		throw std::logic_error("nargs > 4096");
	}
	if(!remote_call) {
		throw std::logic_error("unable to make remote calls");
	}
	gas_used += INSTR_CALL_COST;

	auto frame = get_frame();
	frame.stack_ptr += stack_ptr;
	call_stack.push_back(frame);

	remote_call(read_fail<binary_t>(name, TYPE_STRING).to_string(),
				read_fail<binary_t>(method, TYPE_STRING).to_string(), nargs);
	ret();
}

void Engine::cread(const uint64_t dst, const uint64_t address, const uint64_t field)
{
	if(!read_contract) {
		throw std::logic_error("unable to read contract fields");
	}
	read_contract(
			read_fail<binary_t>(address, TYPE_BINARY).to_addr(),
			read_fail<binary_t>(field, TYPE_STRING).to_string(), dst);
}

uint128_t& Engine::get_balance(const uint64_t currency_addr)
{
	const auto currency = read_fail<binary_t>(currency_addr, TYPE_BINARY).to_addr();
	{
		auto iter = balance_map.find(currency);
		if(iter != balance_map.end()) {
			return iter->second;
		}
	}
	gas_used += WRITE_COST;

	const auto value = storage->get_balance(contract, currency);
	return balance_map[currency] = (value ? *value : uint128_0);
}

void Engine::read_balance(const uint64_t dst, const uint64_t currency)
{
	write(dst, uint_t(get_balance(currency)));
}

void Engine::jump(const uint64_t instr_ptr)
{
	get_frame().instr_ptr = instr_ptr;
}

void Engine::call(const uint64_t instr_ptr, const uint64_t stack_ptr)
{
	if(stack_ptr >= STACK_SIZE) {
		throw std::logic_error("stack overflow");
	}
	if(call_stack.size() >= MAX_CALL_RECURSION) {
		throw std::logic_error("recursion overflow");
	}
	gas_used += INSTR_CALL_COST;

	auto frame = get_frame();
	frame.instr_ptr = instr_ptr;
	frame.stack_ptr += stack_ptr;
	call_stack.push_back(frame);
}

bool Engine::ret()
{
	clear_stack(get_frame().stack_ptr + 1);
	call_stack.pop_back();
	return call_stack.empty();
}

Engine::frame_t& Engine::get_frame()
{
	if(call_stack.empty()) {
		throw std::logic_error("empty call stack");
	}
	return call_stack.back();
}

uint64_t Engine::deref(const uint64_t src)
{
	const auto& var = read_fail(src);
	if(var.type != TYPE_REF) {
		return src;
	}
	return ((const ref_t&)var).address;
}

uint64_t Engine::deref_addr(uint32_t src, const bool flag)
{
	if(src >= MEM_STACK && src < MEM_STATIC) {
		const auto& frame = get_frame();
		if(uint64_t(src) + frame.stack_ptr >= MEM_STATIC) {
			throw std::runtime_error("stack overflow");
		}
		src += frame.stack_ptr;
	}
	return flag ? deref(src) : src;
}

uint64_t Engine::deref_value(uint32_t src, const bool flag)
{
	if(flag) {
		return read_fail<uint_t>(deref_addr(src, false), TYPE_UINT).value;
	}
	return src;
}

void Engine::exec(const instr_t& instr)
{
	gas_used += INSTR_COST;

	switch(instr.code) {
	case OP_NOP:
		break;
	case OP_CLR:
		if(instr.flags & OPFLAG_REF_A) {
			throw std::logic_error("OPFLAG_REF_A not supported");
		}
		erase(instr.a);
		break;
	case OP_COPY:
		copy(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B));
		break;
	case OP_CLONE:
		clone(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B));
		break;
	case OP_JUMP: {
		jump(	deref_value(instr.a, instr.flags & OPFLAG_REF_A));
		return;
	}
	case OP_JUMPI:
	case OP_JUMPN: {
		const auto cond = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto& var = read_fail(cond);
		switch(var.type) {
			case TYPE_TRUE:
			case TYPE_FALSE: break;
			default: throw invalid_type(var);
		}
		if((instr.code == OP_JUMPI) == (var.type == TYPE_TRUE)) {
			jump(deref_value(instr.a, instr.flags & OPFLAG_REF_A));
			return;
		}
		break;
	}
	case OP_CALL:
		if(instr.flags & OPFLAG_REF_B) {
			throw std::logic_error("OPFLAG_REF_B not supported");
		}
		call(	deref_value(instr.a, instr.flags & OPFLAG_REF_A),
				deref_value(instr.b, instr.flags & OPFLAG_REF_B));
		return;
	case OP_RET:
		if(ret()) {
			return;
		}
		break;
	case OP_ADD: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto lhs = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto rhs = deref_addr(instr.c, instr.flags & OPFLAG_REF_C);
		const auto& L = read_fail<uint_t>(lhs, TYPE_UINT).value;
		const auto& R = read_fail<uint_t>(rhs, TYPE_UINT).value;
		const uint256_t D = L + R;
		if((instr.flags & OPFLAG_CATCH_OVERFLOW) && D < L) {
			throw std::runtime_error("integer overflow");
		}
		write(dst, uint_t(D));
		break;
	}
	case OP_SUB: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto lhs = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto rhs = deref_addr(instr.c, instr.flags & OPFLAG_REF_C);
		const auto& L = read_fail<uint_t>(lhs, TYPE_UINT).value;
		const auto& R = read_fail<uint_t>(rhs, TYPE_UINT).value;
		const uint256_t D = L - R;
		if((instr.flags & OPFLAG_CATCH_OVERFLOW) && D > L) {
			throw std::runtime_error("integer overflow");
		}
		write(dst, uint_t(D));
		break;
	}
	case OP_MUL: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto lhs = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto rhs = deref_addr(instr.c, instr.flags & OPFLAG_REF_C);
		const auto& L = read_fail<uint_t>(lhs, TYPE_UINT).value;
		const auto& R = read_fail<uint_t>(rhs, TYPE_UINT).value;
		if((L >> 64) || (R >> 64)) {
			gas_used += INSTR_MUL_256_COST;
		} else {
			gas_used += INSTR_MUL_128_COST;
		}
		const uint256_t D = L * R;
		if((instr.flags & OPFLAG_CATCH_OVERFLOW) && (D < L && D < R)) {
			throw std::runtime_error("integer overflow");
		}
		write(dst, uint_t(D));
		break;
	}
	case OP_DIV: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto lhs = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto rhs = deref_addr(instr.c, instr.flags & OPFLAG_REF_C);
		const auto& L = read_fail<uint_t>(lhs, TYPE_UINT).value;
		const auto& R = read_fail<uint_t>(rhs, TYPE_UINT).value;
		if(R == uint256_0) {
			throw std::runtime_error("division by zero");
		}
		if(L.upper()) {
			gas_used += INSTR_DIV_256_COST;
		} else if(L.lower().upper()) {
			gas_used += INSTR_DIV_128_COST;
		} else {
			gas_used += INSTR_DIV_64_COST;
		}
		write(dst, uint_t(L / R));
		break;
	}
	case OP_MOD: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto lhs = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto rhs = deref_addr(instr.c, instr.flags & OPFLAG_REF_C);
		const auto& L = read_fail<uint_t>(lhs, TYPE_UINT).value;
		const auto& R = read_fail<uint_t>(rhs, TYPE_UINT).value;
		if(R == uint256_0) {
			throw std::runtime_error("division by zero");
		}
		if(L.upper()) {
			gas_used += INSTR_DIV_256_COST;
		} else if(L.lower().upper()) {
			gas_used += INSTR_DIV_128_COST;
		} else {
			gas_used += INSTR_DIV_64_COST;
		}
		write(dst, uint_t(L % R));
		break;
	}
	case OP_NOT: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto src = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto& var = read_fail(src);
		switch(var.type) {
			case TYPE_TRUE: write(dst, var_t(TYPE_FALSE)); break;
			case TYPE_FALSE: write(dst, var_t(TYPE_TRUE)); break;
			case TYPE_UINT:
				write(dst, uint_t(~((const uint_t&)var).value));
				break;
			default:
				throw invalid_type(var);
		}
		break;
	}
	case OP_XOR: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto lhs = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto rhs = deref_addr(instr.c, instr.flags & OPFLAG_REF_C);
		const auto& L = read_fail(lhs);
		const auto& R = read_fail(rhs);
		switch(L.type) {
			case TYPE_TRUE:
				switch(R.type) {
					case TYPE_TRUE: write(dst, var_t(TYPE_FALSE)); break;
					case TYPE_FALSE: write(dst, var_t(TYPE_TRUE)); break;
					default: throw std::runtime_error("type mismatch");
				}
				break;
			case TYPE_FALSE:
				switch(R.type) {
					case TYPE_TRUE: write(dst, var_t(TYPE_TRUE)); break;
					case TYPE_FALSE: write(dst, var_t(TYPE_FALSE)); break;
					default: throw std::runtime_error("type mismatch");
				}
				break;
			case TYPE_UINT:
				if(R.type == TYPE_UINT) {
					write(dst, uint_t(((const uint_t&)L).value ^ ((const uint_t&)R).value));
				} else {
					throw std::runtime_error("type mismatch");
				}
				break;
			default:
				throw invalid_type(L);
		}
		break;
	}
	case OP_AND: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto lhs = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto rhs = deref_addr(instr.c, instr.flags & OPFLAG_REF_C);
		const auto& L = read_fail(lhs);
		const auto& R = read_fail(rhs);
		switch(L.type) {
			case TYPE_TRUE:
				switch(R.type) {
					case TYPE_TRUE: write(dst, var_t(TYPE_TRUE)); break;
					case TYPE_FALSE: write(dst, var_t(TYPE_FALSE)); break;
					default: throw std::runtime_error("type mismatch");
				}
				break;
			case TYPE_FALSE:
				switch(R.type) {
					case TYPE_TRUE:
					case TYPE_FALSE: write(dst, var_t(TYPE_FALSE)); break;
					default: throw std::runtime_error("type mismatch");
				}
				break;
			case TYPE_UINT:
				if(R.type == TYPE_UINT) {
					write(dst, uint_t(((const uint_t&)L).value & ((const uint_t&)R).value));
				} else {
					throw std::runtime_error("type mismatch");
				}
				break;
			default:
				throw invalid_type(L);
		}
		break;
	}
	case OP_OR: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto lhs = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto rhs = deref_addr(instr.c, instr.flags & OPFLAG_REF_C);
		const auto& L = read_fail(lhs);
		const auto& R = read_fail(rhs);
		switch(L.type) {
			case TYPE_TRUE:
				switch(R.type) {
					case TYPE_TRUE:
					case TYPE_FALSE: write(dst, var_t(TYPE_TRUE)); break;
					default: throw std::runtime_error("type mismatch");
				}
				break;
			case TYPE_FALSE:
				switch(R.type) {
					case TYPE_TRUE: write(dst, var_t(TYPE_TRUE)); break;
					case TYPE_FALSE: write(dst, var_t(TYPE_FALSE)); break;
					default: throw std::runtime_error("type mismatch");
				}
				break;
			case TYPE_UINT:
				if(R.type == TYPE_UINT) {
					write(dst, uint_t(((const uint_t&)L).value | ((const uint_t&)R).value));
				} else {
					throw std::runtime_error("type mismatch");
				}
				break;
			default:
				throw invalid_type(L);
		}
		break;
	}
	case OP_MIN: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto lhs = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto rhs = deref_addr(instr.c, instr.flags & OPFLAG_REF_C);
		const auto& L = read_fail<uint_t>(lhs, TYPE_UINT).value;
		const auto& R = read_fail<uint_t>(rhs, TYPE_UINT).value;
		write(dst, uint_t(L < R ? L : R));
		break;
	}
	case OP_MAX: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto lhs = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto rhs = deref_addr(instr.c, instr.flags & OPFLAG_REF_C);
		const auto& L = read_fail<uint_t>(lhs, TYPE_UINT).value;
		const auto& R = read_fail<uint_t>(rhs, TYPE_UINT).value;
		write(dst, uint_t(L > R ? L : R));
		break;
	}
	case OP_SHL: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto src = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto count = deref_value(instr.c, instr.flags & OPFLAG_REF_C);
		const auto& value = read_fail<uint_t>(src, TYPE_UINT).value;
		write(dst, uint_t(value << count));
		break;
	}
	case OP_SHR: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto src = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto count = deref_value(instr.c, instr.flags & OPFLAG_REF_C);
		const auto& value = read_fail<uint_t>(src, TYPE_UINT).value;
		write(dst, uint_t(value >> count));
		break;
	}
	case OP_CMP_EQ:
	case OP_CMP_NEQ:
	case OP_CMP_LT:
	case OP_CMP_GT:
	case OP_CMP_LTE:
	case OP_CMP_GTE: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto lhs = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto rhs = deref_addr(instr.c, instr.flags & OPFLAG_REF_C);
		const auto cmp = compare(read_fail(lhs), read_fail(rhs));
		bool res = false;
		switch(instr.code) {
			case OP_CMP_EQ: res = (cmp == 0); break;
			case OP_CMP_NEQ: res = (cmp != 0); break;
			case OP_CMP_LT: res = (cmp < 0); break;
			case OP_CMP_GT: res = (cmp > 0); break;
			case OP_CMP_LTE: res = (cmp <= 0); break;
			case OP_CMP_GTE: res = (cmp >= 0); break;
			default: break;
		}
		write(dst, var_t(res));
		break;
	}
	case OP_TYPE: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto addr = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto& var = read_fail(addr);
		write(dst, uint_t(uint32_t(var.type)));
		break;
	}
	case OP_SIZE: {
		const auto dst = deref_addr(instr.a, instr.flags & OPFLAG_REF_A);
		const auto addr = deref_addr(instr.b, instr.flags & OPFLAG_REF_B);
		const auto& var = read_fail(addr);
		switch(var.type) {
			case TYPE_STRING:
			case TYPE_BINARY:
				write(dst, uint_t(((const binary_t&)var).size));
				break;
			case TYPE_ARRAY:
				write(dst, uint_t(((const array_t&)var).size));
				break;
			default:
				throw invalid_type(var);
		}
		break;
	}
	case OP_GET:
		get(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B),
				deref_addr(instr.c, instr.flags & OPFLAG_REF_C), instr.flags);
		break;
	case OP_SET:
		set(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B),
				deref_addr(instr.c, instr.flags & OPFLAG_REF_C), instr.flags);
		break;
	case OP_ERASE:
		erase(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B), instr.flags);
		break;
	case OP_PUSH_BACK:
		push_back(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
					deref_addr(instr.b, instr.flags & OPFLAG_REF_B));
		break;
	case OP_POP_BACK:
		pop_back(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
					deref_addr(instr.b, instr.flags & OPFLAG_REF_B));
		break;
	case OP_CONV:
		conv(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B),
				deref_value(instr.c, instr.flags & OPFLAG_REF_C),
				deref_value(instr.d, instr.flags & OPFLAG_REF_D));
		break;
	case OP_CONCAT:
		concat(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B),
				deref_addr(instr.c, instr.flags & OPFLAG_REF_C));
		break;
	case OP_MEMCPY:
		memcpy(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B),
				deref_value(instr.c, instr.flags & OPFLAG_REF_C),
				deref_value(instr.d, instr.flags & OPFLAG_REF_D));
		break;
	case OP_SHA256:
		sha256(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B));
		break;
	case OP_VERIFY:
		verify(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B),
				deref_addr(instr.c, instr.flags & OPFLAG_REF_C),
				deref_addr(instr.d, instr.flags & OPFLAG_REF_D));
		break;
	case OP_LOG:
		log(	deref_value(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B));
		break;
	case OP_SEND:
		send(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B),
				deref_addr(instr.c, instr.flags & OPFLAG_REF_C),
				deref_addr(instr.d, instr.flags & OPFLAG_REF_D));
		break;
	case OP_MINT:
		mint(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B),
				deref_addr(instr.c, instr.flags & OPFLAG_REF_C));
		break;
	case OP_EVENT:
		event(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B));
		break;
	case OP_FAIL:
		error_code = deref_value(instr.b, instr.flags & OPFLAG_REF_B);
		throw std::runtime_error("failed with: "
				+ to_string_value(read(deref_addr(instr.a, instr.flags & OPFLAG_REF_A))));
	case OP_RCALL:
		rcall(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B),
				deref_value(instr.c, instr.flags & OPFLAG_REF_C),
				deref_value(instr.d, instr.flags & OPFLAG_REF_D));
		break;
	case OP_CREAD:
		cread(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
				deref_addr(instr.b, instr.flags & OPFLAG_REF_B),
				deref_addr(instr.c, instr.flags & OPFLAG_REF_C));
		break;
	case OP_BALANCE:
		read_balance(	deref_addr(instr.a, instr.flags & OPFLAG_REF_A),
						deref_addr(instr.b, instr.flags & OPFLAG_REF_B));
		break;
	default:
		throw std::logic_error("invalid op_code: " + to_hex(uint32_t(instr.code)));
	}
	get_frame().instr_ptr++;
}

void Engine::clear_stack(const uint64_t offset)
{
	for(auto iter = memory.lower_bound(MEM_STACK + offset); iter != memory.lower_bound(MEM_STATIC);) {
		clear(iter->second.get());
		iter = memory.erase(iter);
		check_gas();
	}
}

void Engine::commit()
{
	clear_stack();	// clear references from stack

	for(auto iter = memory.lower_bound(MEM_STATIC); iter != memory.end(); ++iter)
	{
		if(auto var = iter->second.get()) {
			if(var->flags & FLAG_DIRTY) {
				storage->write(contract, iter->first, *var);
				var->flags &= ~FLAG_DIRTY;
			}
		}
	}
	for(auto iter = entries.lower_bound(std::make_pair(MEM_STATIC, 0)); iter != entries.end(); ++iter)
	{
		if(auto var = iter->second.get()) {
			if(var->flags & FLAG_DIRTY) {
				storage->write(contract, iter->first.first, iter->first.second, *var);
				var->flags &= ~FLAG_DIRTY;
			}
		}
	}
	check_gas();
}

std::map<uint64_t, const var_t*> Engine::find_entries(const uint64_t dst) const
{
	std::map<uint64_t, const var_t*> out;
	const auto begin = entries.lower_bound(std::make_pair(dst, 0));
	for(auto iter = begin; iter != entries.end() && iter->first.first == dst; ++iter) {
		if(auto var = iter->second.get()) {
			if((var->flags & FLAG_DELETED) == 0) {
				out[iter->first.second] = var;
			}
		}
	}
	return out;
}

void Engine::dump_memory(const uint64_t begin, const uint64_t end)
{
	for(auto iter = memory.lower_bound(begin); iter != memory.lower_bound(end); ++iter) {
		std::cout << "[" << to_hex(iter->first) << "] " << to_string(iter->second.get());
		if(auto var = iter->second.get()) {
			std::cout << "\t\t(vf: " << to_bin(var->flags) << ") (rc: " << var->ref_count << ")";
		}
		std::cout << std::endl;
	}
	for(auto iter = entries.lower_bound(std::make_pair(begin, 0)); iter != entries.lower_bound(std::make_pair(end, 0)); ++iter) {
		std::cout << "[" << to_hex(iter->first.first) << "]"
				<< "[" << to_hex(iter->first.second) << "] " << to_string(iter->second.get());
		if(auto var = iter->second.get()) {
			std::cout << "\t\t(vf: " << to_bin(var->flags) << ")";
		}
		std::cout << std::endl;
	}
}


} // vm
} // mmx

/*
 * storage_tests.cpp
 *
 *  Created on: Apr 23, 2024
 *      Author: mad
 */

#include <mmx/vm/Engine.h>
#include <mmx/vm/StorageDB.h>
#include <mmx/vm/StorageRAM.h>
#include <mmx/vm/StorageCache.h>
#include <mmx/vm_interface.h>

#include <vnx/vnx.h>
#include <vnx/test/Test.h>

#include <iostream>

using namespace mmx;


void expect(const vm::var_t* got, const vm::var_t* want) {
	if(compare(got, want)) {
		throw std::logic_error("expected " + to_string(want) + " but got " + to_string(got));
	}
}

void expect(const vm::var_t* got, const vm::var_t& want) {
	expect(got, &want);
}

void expect(const vm::var_t* got, vm::varptr_t want) {
	expect(got, want.get());
}

std::shared_ptr<vm::Engine> new_engine(std::shared_ptr<vm::Storage> storage, bool read_only)
{
	auto engine = std::make_shared<vm::Engine>(addr_t(), storage, read_only);
	engine->gas_limit = 1000000;
	engine->init();
	return engine;
}


int main(int argc, char** argv)
{
	vnx::test::init("mmx.vm.storage");

	auto db = std::make_shared<mmx::DataBase>(1);
	auto storage = std::make_shared<vm::StorageDB>("tmp/", db);

	VNX_TEST_BEGIN("stack")
	{
		db->revert(0);
		{
			auto engine = new_engine(storage, false);
			engine->write(vm::MEM_STACK, vm::uint_t(1337));
			engine->commit();
		}
		db->commit(1);
		{
			auto engine = new_engine(storage, true);
			expect(engine->read(vm::MEM_STACK), nullptr);
		}
	}
	VNX_TEST_END()

	VNX_TEST_BEGIN("heap")
	{
		db->revert(0);
		uint64_t addr = 0;
		{
			auto engine = new_engine(storage, false);
			addr = engine->alloc();
			engine->write(addr, vm::uint_t(1337));
			engine->write(vm::MEM_STATIC, vm::ref_t(addr));
			engine->commit();
		}
		db->commit(1);
		{
			auto engine = new_engine(storage, true);
			const auto ref = engine->read(vm::MEM_STATIC);
			vnx::test::expect(bool(ref), true);
			const auto value = engine->read(vm::get_address(ref));
			expect(value, vm::uint_t(1337));
		}
		{
			auto engine = new_engine(storage, false);
			engine->erase(vm::MEM_STATIC);
			engine->commit();
		}
		db->commit(2);
		{
			auto engine = new_engine(storage, true);
			expect(engine->read(vm::MEM_STATIC), nullptr);
			expect(engine->read(addr), vm::uint_t(1337));
		}
	}
	VNX_TEST_END()

	VNX_TEST_BEGIN("heap_erase")
	{
		db->revert(0);
		uint64_t addr = 0;
		uint64_t addr2 = 0;
		{
			auto engine = new_engine(storage, false);
			addr = engine->alloc();
			engine->write(addr, vm::uint_t(1337));
			engine->write(vm::MEM_STATIC, vm::ref_t(addr));
			expect(engine->read(addr), vm::uint_t(1337));

			addr2 = engine->alloc();
			engine->write(addr2, vm::uint_t(11));
			engine->write(vm::MEM_STATIC, vm::ref_t(addr2));
			expect(engine->read(addr), nullptr);

			engine->erase(vm::MEM_STATIC);
			expect(engine->read(vm::MEM_STATIC), nullptr);
			expect(engine->read(addr2), nullptr);
			engine->commit();
		}
		db->commit(1);
		{
			auto engine = new_engine(storage, true);
			expect(engine->read(vm::MEM_STATIC), nullptr);
			expect(engine->read(addr), nullptr);
			expect(engine->read(addr2), nullptr);
			engine->commit();
		}
	}
	VNX_TEST_END()

	VNX_TEST_BEGIN("lookup")
	{
		db->revert(0);
		{
			auto engine = new_engine(storage, false);
			const auto key = engine->lookup(vm::to_binary("test"), false);
			engine->write(vm::MEM_STATIC, vm::map_t());
			vnx::test::expect(vm::get_address(engine->read(vm::MEM_STATIC)), vm::MEM_STATIC);
			engine->write_key(vm::MEM_STATIC, key, vm::uint_t(1337));
			engine->commit();
		}
		db->commit(1);
		{
			auto engine = new_engine(storage, true);
			const auto key = engine->lookup(vm::to_binary("test"), true);
			const auto value = engine->read_key(vm::MEM_STATIC, key);
			expect(value, vm::uint_t(1337));
		}
	}
	VNX_TEST_END()

	VNX_TEST_BEGIN("over_write")
	{
		db->revert(0);
		{
			auto engine = new_engine(storage, false);
			engine->write(vm::MEM_STATIC, vm::uint_t(1337));
			engine->commit();
		}
		db->commit(1);
		{
			auto engine = new_engine(storage, false);
			engine->write(vm::MEM_STATIC, vm::uint_t(11337));
			engine->commit();
		}
		db->commit(2);
		{
			auto engine = new_engine(storage, true);
			const auto value = engine->read(vm::MEM_STATIC);
			expect(value, vm::uint_t(11337));
		}
	}
	VNX_TEST_END()

	VNX_TEST_BEGIN("push_pop_back")
	{
		db->revert(0);
		{
			auto engine = new_engine(storage, false);
			engine->write(vm::MEM_STATIC, vm::array_t());
			vnx::test::expect(vm::get_address(engine->read(vm::MEM_STATIC)), vm::MEM_STATIC);
			engine->push_back(vm::MEM_STATIC, vm::uint_t(1));
			engine->commit();
		}
		db->commit(1);
		{
			auto engine = new_engine(storage, false);
			vnx::test::expect(vm::get_address(engine->read(vm::MEM_STATIC)), vm::MEM_STATIC);
			engine->push_back(vm::MEM_STATIC, vm::uint_t(2));
			engine->commit();
		}
		db->commit(2);
		{
			auto engine = new_engine(storage, true);
			const auto value = engine->read(vm::MEM_STATIC);
			vnx::test::expect(bool(value), true);
			vnx::test::expect(value->type, vm::TYPE_ARRAY);
			auto array = (vm::array_t*)value;
			vnx::test::expect(array->size, 2u);
			expect(engine->read_entry(vm::MEM_STATIC, 0), vm::uint_t(1));
			expect(engine->read_entry(vm::MEM_STATIC, 1), vm::uint_t(2));
		}
		{
			auto engine = new_engine(storage, false);
			vnx::test::expect(vm::get_address(engine->read(vm::MEM_STATIC)), vm::MEM_STATIC);
			engine->pop_back(vm::MEM_STACK, vm::MEM_STATIC);
			expect(engine->read(vm::MEM_STACK), vm::uint_t(2));
			engine->commit();
		}
		db->commit(3);
		{
			auto engine = new_engine(storage, true);
			const auto value = engine->read(vm::MEM_STATIC);
			vnx::test::expect(bool(value), true);
			vnx::test::expect(value->type, vm::TYPE_ARRAY);
			auto array = (vm::array_t*)value;
			vnx::test::expect(array->size, 1u);
			expect(engine->read_entry(vm::MEM_STATIC, 0), vm::uint_t(1));
		}
	}
	VNX_TEST_END()

	// TODO: more

	return vnx::test::done();
}


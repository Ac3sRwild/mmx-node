
// AUTO GENERATED by vnxcppcodegen

#include <mmx/package.hxx>
#include <mmx/Challenge.hxx>
#include <mmx/Challenge_calc_hash.hxx>
#include <mmx/Challenge_calc_hash_return.hxx>
#include <mmx/hash_t.hpp>
#include <vnx/Value.h>

#include <vnx/vnx.h>


namespace mmx {


const vnx::Hash64 Challenge::VNX_TYPE_HASH(0x4bf49f8022405249ull);
const vnx::Hash64 Challenge::VNX_CODE_HASH(0x7977515814dd0f3eull);

vnx::Hash64 Challenge::get_type_hash() const {
	return VNX_TYPE_HASH;
}

std::string Challenge::get_type_name() const {
	return "mmx.Challenge";
}

const vnx::TypeCode* Challenge::get_type_code() const {
	return mmx::vnx_native_type_code_Challenge;
}

std::shared_ptr<Challenge> Challenge::create() {
	return std::make_shared<Challenge>();
}

std::shared_ptr<vnx::Value> Challenge::clone() const {
	return std::make_shared<Challenge>(*this);
}

void Challenge::read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) {
	vnx::read(_in, *this, _type_code, _code);
}

void Challenge::write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const {
	vnx::write(_out, *this, _type_code, _code);
}

void Challenge::accept(vnx::Visitor& _visitor) const {
	const vnx::TypeCode* _type_code = mmx::vnx_native_type_code_Challenge;
	_visitor.type_begin(*_type_code);
	_visitor.type_field(_type_code->fields[0], 0); vnx::accept(_visitor, height);
	_visitor.type_field(_type_code->fields[1], 1); vnx::accept(_visitor, challenge);
	_visitor.type_field(_type_code->fields[2], 2); vnx::accept(_visitor, diff_block_hash);
	_visitor.type_field(_type_code->fields[3], 3); vnx::accept(_visitor, space_diff);
	_visitor.type_field(_type_code->fields[4], 4); vnx::accept(_visitor, max_delay);
	_visitor.type_end(*_type_code);
}

void Challenge::write(std::ostream& _out) const {
	_out << "{\"__type\": \"mmx.Challenge\"";
	_out << ", \"height\": "; vnx::write(_out, height);
	_out << ", \"challenge\": "; vnx::write(_out, challenge);
	_out << ", \"diff_block_hash\": "; vnx::write(_out, diff_block_hash);
	_out << ", \"space_diff\": "; vnx::write(_out, space_diff);
	_out << ", \"max_delay\": "; vnx::write(_out, max_delay);
	_out << "}";
}

void Challenge::read(std::istream& _in) {
	if(auto _json = vnx::read_json(_in)) {
		from_object(_json->to_object());
	}
}

vnx::Object Challenge::to_object() const {
	vnx::Object _object;
	_object["__type"] = "mmx.Challenge";
	_object["height"] = height;
	_object["challenge"] = challenge;
	_object["diff_block_hash"] = diff_block_hash;
	_object["space_diff"] = space_diff;
	_object["max_delay"] = max_delay;
	return _object;
}

void Challenge::from_object(const vnx::Object& _object) {
	for(const auto& _entry : _object.field) {
		if(_entry.first == "challenge") {
			_entry.second.to(challenge);
		} else if(_entry.first == "diff_block_hash") {
			_entry.second.to(diff_block_hash);
		} else if(_entry.first == "height") {
			_entry.second.to(height);
		} else if(_entry.first == "max_delay") {
			_entry.second.to(max_delay);
		} else if(_entry.first == "space_diff") {
			_entry.second.to(space_diff);
		}
	}
}

vnx::Variant Challenge::get_field(const std::string& _name) const {
	if(_name == "height") {
		return vnx::Variant(height);
	}
	if(_name == "challenge") {
		return vnx::Variant(challenge);
	}
	if(_name == "diff_block_hash") {
		return vnx::Variant(diff_block_hash);
	}
	if(_name == "space_diff") {
		return vnx::Variant(space_diff);
	}
	if(_name == "max_delay") {
		return vnx::Variant(max_delay);
	}
	return vnx::Variant();
}

void Challenge::set_field(const std::string& _name, const vnx::Variant& _value) {
	if(_name == "height") {
		_value.to(height);
	} else if(_name == "challenge") {
		_value.to(challenge);
	} else if(_name == "diff_block_hash") {
		_value.to(diff_block_hash);
	} else if(_name == "space_diff") {
		_value.to(space_diff);
	} else if(_name == "max_delay") {
		_value.to(max_delay);
	}
}

/// \private
std::ostream& operator<<(std::ostream& _out, const Challenge& _value) {
	_value.write(_out);
	return _out;
}

/// \private
std::istream& operator>>(std::istream& _in, Challenge& _value) {
	_value.read(_in);
	return _in;
}

const vnx::TypeCode* Challenge::static_get_type_code() {
	const vnx::TypeCode* type_code = vnx::get_type_code(VNX_TYPE_HASH);
	if(!type_code) {
		type_code = vnx::register_type_code(static_create_type_code());
	}
	return type_code;
}

std::shared_ptr<vnx::TypeCode> Challenge::static_create_type_code() {
	auto type_code = std::make_shared<vnx::TypeCode>();
	type_code->name = "mmx.Challenge";
	type_code->type_hash = vnx::Hash64(0x4bf49f8022405249ull);
	type_code->code_hash = vnx::Hash64(0x7977515814dd0f3eull);
	type_code->is_native = true;
	type_code->is_class = true;
	type_code->native_size = sizeof(::mmx::Challenge);
	type_code->create_value = []() -> std::shared_ptr<vnx::Value> { return std::make_shared<Challenge>(); };
	type_code->methods.resize(1);
	type_code->methods[0] = ::mmx::Challenge_calc_hash::static_get_type_code();
	type_code->fields.resize(5);
	{
		auto& field = type_code->fields[0];
		field.data_size = 4;
		field.name = "height";
		field.code = {3};
	}
	{
		auto& field = type_code->fields[1];
		field.is_extended = true;
		field.name = "challenge";
		field.code = {11, 32, 1};
	}
	{
		auto& field = type_code->fields[2];
		field.is_extended = true;
		field.name = "diff_block_hash";
		field.code = {11, 32, 1};
	}
	{
		auto& field = type_code->fields[3];
		field.data_size = 8;
		field.name = "space_diff";
		field.code = {4};
	}
	{
		auto& field = type_code->fields[4];
		field.data_size = 4;
		field.name = "max_delay";
		field.code = {3};
	}
	type_code->build();
	return type_code;
}

std::shared_ptr<vnx::Value> Challenge::vnx_call_switch(std::shared_ptr<const vnx::Value> _method) {
	switch(_method->get_type_hash()) {
		case 0xfd336f4fe0150c72ull: {
			auto _args = std::static_pointer_cast<const ::mmx::Challenge_calc_hash>(_method);
			auto _return_value = ::mmx::Challenge_calc_hash_return::create();
			_return_value->_ret_0 = calc_hash();
			return _return_value;
		}
	}
	return nullptr;
}


} // namespace mmx


namespace vnx {

void read(TypeInput& in, ::mmx::Challenge& value, const TypeCode* type_code, const uint16_t* code) {
	if(code) {
		switch(code[0]) {
			case CODE_OBJECT:
			case CODE_ALT_OBJECT: {
				Object tmp;
				vnx::read(in, tmp, type_code, code);
				value.from_object(tmp);
				return;
			}
			case CODE_DYNAMIC:
			case CODE_ALT_DYNAMIC:
				vnx::read_dynamic(in, value);
				return;
		}
	}
	if(!type_code) {
		vnx::skip(in, type_code, code);
		return;
	}
	if(code) {
		switch(code[0]) {
			case CODE_STRUCT: type_code = type_code->depends[code[1]]; break;
			case CODE_ALT_STRUCT: type_code = type_code->depends[vnx::flip_bytes(code[1])]; break;
			default: {
				vnx::skip(in, type_code, code);
				return;
			}
		}
	}
	const char* const _buf = in.read(type_code->total_field_size);
	if(type_code->is_matched) {
		if(const auto* const _field = type_code->field_map[0]) {
			vnx::read_value(_buf + _field->offset, value.height, _field->code.data());
		}
		if(const auto* const _field = type_code->field_map[3]) {
			vnx::read_value(_buf + _field->offset, value.space_diff, _field->code.data());
		}
		if(const auto* const _field = type_code->field_map[4]) {
			vnx::read_value(_buf + _field->offset, value.max_delay, _field->code.data());
		}
	}
	for(const auto* _field : type_code->ext_fields) {
		switch(_field->native_index) {
			case 1: vnx::read(in, value.challenge, type_code, _field->code.data()); break;
			case 2: vnx::read(in, value.diff_block_hash, type_code, _field->code.data()); break;
			default: vnx::skip(in, type_code, _field->code.data());
		}
	}
}

void write(TypeOutput& out, const ::mmx::Challenge& value, const TypeCode* type_code, const uint16_t* code) {
	if(code && code[0] == CODE_OBJECT) {
		vnx::write(out, value.to_object(), nullptr, code);
		return;
	}
	if(!type_code || (code && code[0] == CODE_ANY)) {
		type_code = mmx::vnx_native_type_code_Challenge;
		out.write_type_code(type_code);
		vnx::write_class_header<::mmx::Challenge>(out);
	}
	else if(code && code[0] == CODE_STRUCT) {
		type_code = type_code->depends[code[1]];
	}
	char* const _buf = out.write(16);
	vnx::write_value(_buf + 0, value.height);
	vnx::write_value(_buf + 4, value.space_diff);
	vnx::write_value(_buf + 12, value.max_delay);
	vnx::write(out, value.challenge, type_code, type_code->fields[1].code.data());
	vnx::write(out, value.diff_block_hash, type_code, type_code->fields[2].code.data());
}

void read(std::istream& in, ::mmx::Challenge& value) {
	value.read(in);
}

void write(std::ostream& out, const ::mmx::Challenge& value) {
	value.write(out);
}

void accept(Visitor& visitor, const ::mmx::Challenge& value) {
	value.accept(visitor);
}

} // vnx

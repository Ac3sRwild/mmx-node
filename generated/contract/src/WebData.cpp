
// AUTO GENERATED by vnxcppcodegen

#include <mmx/contract/package.hxx>
#include <mmx/contract/WebData.hxx>
#include <mmx/ChainParams.hxx>
#include <mmx/Contract.hxx>
#include <mmx/Contract_calc_cost.hxx>
#include <mmx/Contract_calc_cost_return.hxx>
#include <mmx/Contract_calc_hash.hxx>
#include <mmx/Contract_calc_hash_return.hxx>
#include <mmx/Contract_get_owner.hxx>
#include <mmx/Contract_get_owner_return.hxx>
#include <mmx/Contract_is_locked.hxx>
#include <mmx/Contract_is_locked_return.hxx>
#include <mmx/Contract_is_valid.hxx>
#include <mmx/Contract_is_valid_return.hxx>
#include <mmx/Contract_read_field.hxx>
#include <mmx/Contract_read_field_return.hxx>
#include <mmx/Contract_validate.hxx>
#include <mmx/Contract_validate_return.hxx>
#include <mmx/addr_t.hpp>
#include <mmx/contract/WebData_calc_cost.hxx>
#include <mmx/contract/WebData_calc_cost_return.hxx>
#include <mmx/contract/WebData_calc_hash.hxx>
#include <mmx/contract/WebData_calc_hash_return.hxx>
#include <mmx/contract/WebData_is_valid.hxx>
#include <mmx/contract/WebData_is_valid_return.hxx>
#include <mmx/contract/WebData_num_bytes.hxx>
#include <mmx/contract/WebData_num_bytes_return.hxx>
#include <mmx/contract/WebData_read_field.hxx>
#include <mmx/contract/WebData_read_field_return.hxx>
#include <mmx/hash_t.hpp>
#include <vnx/Buffer.hpp>
#include <vnx/Variant.hpp>

#include <vnx/vnx.h>


namespace mmx {
namespace contract {


const vnx::Hash64 WebData::VNX_TYPE_HASH(0xf7c226b211c088c4ull);
const vnx::Hash64 WebData::VNX_CODE_HASH(0x1f49299db4776797ull);

vnx::Hash64 WebData::get_type_hash() const {
	return VNX_TYPE_HASH;
}

std::string WebData::get_type_name() const {
	return "mmx.contract.WebData";
}

const vnx::TypeCode* WebData::get_type_code() const {
	return mmx::contract::vnx_native_type_code_WebData;
}

std::shared_ptr<WebData> WebData::create() {
	return std::make_shared<WebData>();
}

std::shared_ptr<vnx::Value> WebData::clone() const {
	return std::make_shared<WebData>(*this);
}

void WebData::read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) {
	vnx::read(_in, *this, _type_code, _code);
}

void WebData::write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const {
	vnx::write(_out, *this, _type_code, _code);
}

void WebData::accept(vnx::Visitor& _visitor) const {
	const vnx::TypeCode* _type_code = mmx::contract::vnx_native_type_code_WebData;
	_visitor.type_begin(*_type_code);
	_visitor.type_field(_type_code->fields[0], 0); vnx::accept(_visitor, version);
	_visitor.type_field(_type_code->fields[1], 1); vnx::accept(_visitor, mime_type);
	_visitor.type_field(_type_code->fields[2], 2); vnx::accept(_visitor, payload);
	_visitor.type_end(*_type_code);
}

void WebData::write(std::ostream& _out) const {
	_out << "{\"__type\": \"mmx.contract.WebData\"";
	_out << ", \"version\": "; vnx::write(_out, version);
	_out << ", \"mime_type\": "; vnx::write(_out, mime_type);
	_out << ", \"payload\": "; vnx::write(_out, payload);
	_out << "}";
}

void WebData::read(std::istream& _in) {
	if(auto _json = vnx::read_json(_in)) {
		from_object(_json->to_object());
	}
}

vnx::Object WebData::to_object() const {
	vnx::Object _object;
	_object["__type"] = "mmx.contract.WebData";
	_object["version"] = version;
	_object["mime_type"] = mime_type;
	_object["payload"] = payload;
	return _object;
}

void WebData::from_object(const vnx::Object& _object) {
	for(const auto& _entry : _object.field) {
		if(_entry.first == "mime_type") {
			_entry.second.to(mime_type);
		} else if(_entry.first == "payload") {
			_entry.second.to(payload);
		} else if(_entry.first == "version") {
			_entry.second.to(version);
		}
	}
}

vnx::Variant WebData::get_field(const std::string& _name) const {
	if(_name == "version") {
		return vnx::Variant(version);
	}
	if(_name == "mime_type") {
		return vnx::Variant(mime_type);
	}
	if(_name == "payload") {
		return vnx::Variant(payload);
	}
	return vnx::Variant();
}

void WebData::set_field(const std::string& _name, const vnx::Variant& _value) {
	if(_name == "version") {
		_value.to(version);
	} else if(_name == "mime_type") {
		_value.to(mime_type);
	} else if(_name == "payload") {
		_value.to(payload);
	}
}

/// \private
std::ostream& operator<<(std::ostream& _out, const WebData& _value) {
	_value.write(_out);
	return _out;
}

/// \private
std::istream& operator>>(std::istream& _in, WebData& _value) {
	_value.read(_in);
	return _in;
}

const vnx::TypeCode* WebData::static_get_type_code() {
	const vnx::TypeCode* type_code = vnx::get_type_code(VNX_TYPE_HASH);
	if(!type_code) {
		type_code = vnx::register_type_code(static_create_type_code());
	}
	return type_code;
}

std::shared_ptr<vnx::TypeCode> WebData::static_create_type_code() {
	auto type_code = std::make_shared<vnx::TypeCode>();
	type_code->name = "mmx.contract.WebData";
	type_code->type_hash = vnx::Hash64(0xf7c226b211c088c4ull);
	type_code->code_hash = vnx::Hash64(0x1f49299db4776797ull);
	type_code->is_native = true;
	type_code->is_class = true;
	type_code->native_size = sizeof(::mmx::contract::WebData);
	type_code->parents.resize(1);
	type_code->parents[0] = ::mmx::Contract::static_get_type_code();
	type_code->create_value = []() -> std::shared_ptr<vnx::Value> { return std::make_shared<WebData>(); };
	type_code->methods.resize(12);
	type_code->methods[0] = ::mmx::Contract_calc_cost::static_get_type_code();
	type_code->methods[1] = ::mmx::Contract_calc_hash::static_get_type_code();
	type_code->methods[2] = ::mmx::Contract_get_owner::static_get_type_code();
	type_code->methods[3] = ::mmx::Contract_is_locked::static_get_type_code();
	type_code->methods[4] = ::mmx::Contract_is_valid::static_get_type_code();
	type_code->methods[5] = ::mmx::Contract_read_field::static_get_type_code();
	type_code->methods[6] = ::mmx::Contract_validate::static_get_type_code();
	type_code->methods[7] = ::mmx::contract::WebData_calc_cost::static_get_type_code();
	type_code->methods[8] = ::mmx::contract::WebData_calc_hash::static_get_type_code();
	type_code->methods[9] = ::mmx::contract::WebData_is_valid::static_get_type_code();
	type_code->methods[10] = ::mmx::contract::WebData_num_bytes::static_get_type_code();
	type_code->methods[11] = ::mmx::contract::WebData_read_field::static_get_type_code();
	type_code->fields.resize(3);
	{
		auto& field = type_code->fields[0];
		field.data_size = 4;
		field.name = "version";
		field.code = {3};
	}
	{
		auto& field = type_code->fields[1];
		field.is_extended = true;
		field.name = "mime_type";
		field.code = {32};
	}
	{
		auto& field = type_code->fields[2];
		field.is_extended = true;
		field.name = "payload";
		field.code = {12, 1};
	}
	type_code->build();
	return type_code;
}

std::shared_ptr<vnx::Value> WebData::vnx_call_switch(std::shared_ptr<const vnx::Value> _method) {
	switch(_method->get_type_hash()) {
		case 0xb23d047adf8b2612ull: {
			auto _args = std::static_pointer_cast<const ::mmx::Contract_calc_cost>(_method);
			auto _return_value = ::mmx::Contract_calc_cost_return::create();
			_return_value->_ret_0 = calc_cost(_args->params, _args->is_read);
			return _return_value;
		}
		case 0x622fcf1cba1952edull: {
			auto _args = std::static_pointer_cast<const ::mmx::Contract_calc_hash>(_method);
			auto _return_value = ::mmx::Contract_calc_hash_return::create();
			_return_value->_ret_0 = calc_hash(_args->full_hash);
			return _return_value;
		}
		case 0x8fe2c64fdc8f0680ull: {
			auto _args = std::static_pointer_cast<const ::mmx::Contract_get_owner>(_method);
			auto _return_value = ::mmx::Contract_get_owner_return::create();
			_return_value->_ret_0 = get_owner();
			return _return_value;
		}
		case 0x9b7981d03b3aeab6ull: {
			auto _args = std::static_pointer_cast<const ::mmx::Contract_is_locked>(_method);
			auto _return_value = ::mmx::Contract_is_locked_return::create();
			_return_value->_ret_0 = is_locked(_args->height);
			return _return_value;
		}
		case 0xe3adf9b29a723217ull: {
			auto _args = std::static_pointer_cast<const ::mmx::Contract_is_valid>(_method);
			auto _return_value = ::mmx::Contract_is_valid_return::create();
			_return_value->_ret_0 = is_valid();
			return _return_value;
		}
		case 0xeff036bd3bb1c0ull: {
			auto _args = std::static_pointer_cast<const ::mmx::Contract_read_field>(_method);
			auto _return_value = ::mmx::Contract_read_field_return::create();
			_return_value->_ret_0 = read_field(_args->name);
			return _return_value;
		}
		case 0xc2126a44901c8d52ull: {
			auto _args = std::static_pointer_cast<const ::mmx::Contract_validate>(_method);
			auto _return_value = ::mmx::Contract_validate_return::create();
			validate(_args->solution, _args->txid);
			return _return_value;
		}
		case 0x3bbbd77da38a1013ull: {
			auto _args = std::static_pointer_cast<const ::mmx::contract::WebData_calc_cost>(_method);
			auto _return_value = ::mmx::contract::WebData_calc_cost_return::create();
			_return_value->_ret_0 = calc_cost(_args->params, _args->is_read);
			return _return_value;
		}
		case 0xeba91c1bc61864ecull: {
			auto _args = std::static_pointer_cast<const ::mmx::contract::WebData_calc_hash>(_method);
			auto _return_value = ::mmx::contract::WebData_calc_hash_return::create();
			_return_value->_ret_0 = calc_hash(_args->full_hash);
			return _return_value;
		}
		case 0x7ae99137e8822105ull: {
			auto _args = std::static_pointer_cast<const ::mmx::contract::WebData_is_valid>(_method);
			auto _return_value = ::mmx::contract::WebData_is_valid_return::create();
			_return_value->_ret_0 = is_valid();
			return _return_value;
		}
		case 0xcc1f554d1bf66504ull: {
			auto _args = std::static_pointer_cast<const ::mmx::contract::WebData_num_bytes>(_method);
			auto _return_value = ::mmx::contract::WebData_num_bytes_return::create();
			_return_value->_ret_0 = num_bytes();
			return _return_value;
		}
		case 0xb3483a5bb9e0ef99ull: {
			auto _args = std::static_pointer_cast<const ::mmx::contract::WebData_read_field>(_method);
			auto _return_value = ::mmx::contract::WebData_read_field_return::create();
			_return_value->_ret_0 = read_field(_args->name);
			return _return_value;
		}
	}
	return nullptr;
}


} // namespace mmx
} // namespace contract


namespace vnx {

void read(TypeInput& in, ::mmx::contract::WebData& value, const TypeCode* type_code, const uint16_t* code) {
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
			vnx::read_value(_buf + _field->offset, value.version, _field->code.data());
		}
	}
	for(const auto* _field : type_code->ext_fields) {
		switch(_field->native_index) {
			case 1: vnx::read(in, value.mime_type, type_code, _field->code.data()); break;
			case 2: vnx::read(in, value.payload, type_code, _field->code.data()); break;
			default: vnx::skip(in, type_code, _field->code.data());
		}
	}
}

void write(TypeOutput& out, const ::mmx::contract::WebData& value, const TypeCode* type_code, const uint16_t* code) {
	if(code && code[0] == CODE_OBJECT) {
		vnx::write(out, value.to_object(), nullptr, code);
		return;
	}
	if(!type_code || (code && code[0] == CODE_ANY)) {
		type_code = mmx::contract::vnx_native_type_code_WebData;
		out.write_type_code(type_code);
		vnx::write_class_header<::mmx::contract::WebData>(out);
	}
	else if(code && code[0] == CODE_STRUCT) {
		type_code = type_code->depends[code[1]];
	}
	char* const _buf = out.write(4);
	vnx::write_value(_buf + 0, value.version);
	vnx::write(out, value.mime_type, type_code, type_code->fields[1].code.data());
	vnx::write(out, value.payload, type_code, type_code->fields[2].code.data());
}

void read(std::istream& in, ::mmx::contract::WebData& value) {
	value.read(in);
}

void write(std::ostream& out, const ::mmx::contract::WebData& value) {
	value.write(out);
}

void accept(Visitor& visitor, const ::mmx::contract::WebData& value) {
	value.accept(visitor);
}

} // vnx

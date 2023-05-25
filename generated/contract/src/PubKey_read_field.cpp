
// AUTO GENERATED by vnxcppcodegen

#include <mmx/contract/package.hxx>
#include <mmx/contract/PubKey_read_field.hxx>
#include <mmx/contract/PubKey_read_field_return.hxx>
#include <vnx/Value.h>

#include <vnx/vnx.h>


namespace mmx {
namespace contract {


const vnx::Hash64 PubKey_read_field::VNX_TYPE_HASH(0x8178b9776d46e140ull);
const vnx::Hash64 PubKey_read_field::VNX_CODE_HASH(0xa291725d0a1505deull);

vnx::Hash64 PubKey_read_field::get_type_hash() const {
	return VNX_TYPE_HASH;
}

std::string PubKey_read_field::get_type_name() const {
	return "mmx.contract.PubKey.read_field";
}

const vnx::TypeCode* PubKey_read_field::get_type_code() const {
	return mmx::contract::vnx_native_type_code_PubKey_read_field;
}

std::shared_ptr<PubKey_read_field> PubKey_read_field::create() {
	return std::make_shared<PubKey_read_field>();
}

std::shared_ptr<vnx::Value> PubKey_read_field::clone() const {
	return std::make_shared<PubKey_read_field>(*this);
}

void PubKey_read_field::read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) {
	vnx::read(_in, *this, _type_code, _code);
}

void PubKey_read_field::write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const {
	vnx::write(_out, *this, _type_code, _code);
}

void PubKey_read_field::accept(vnx::Visitor& _visitor) const {
	const vnx::TypeCode* _type_code = mmx::contract::vnx_native_type_code_PubKey_read_field;
	_visitor.type_begin(*_type_code);
	_visitor.type_field(_type_code->fields[0], 0); vnx::accept(_visitor, name);
	_visitor.type_end(*_type_code);
}

void PubKey_read_field::write(std::ostream& _out) const {
	_out << "{\"__type\": \"mmx.contract.PubKey.read_field\"";
	_out << ", \"name\": "; vnx::write(_out, name);
	_out << "}";
}

void PubKey_read_field::read(std::istream& _in) {
	if(auto _json = vnx::read_json(_in)) {
		from_object(_json->to_object());
	}
}

vnx::Object PubKey_read_field::to_object() const {
	vnx::Object _object;
	_object["__type"] = "mmx.contract.PubKey.read_field";
	_object["name"] = name;
	return _object;
}

void PubKey_read_field::from_object(const vnx::Object& _object) {
	for(const auto& _entry : _object.field) {
		if(_entry.first == "name") {
			_entry.second.to(name);
		}
	}
}

vnx::Variant PubKey_read_field::get_field(const std::string& _name) const {
	if(_name == "name") {
		return vnx::Variant(name);
	}
	return vnx::Variant();
}

void PubKey_read_field::set_field(const std::string& _name, const vnx::Variant& _value) {
	if(_name == "name") {
		_value.to(name);
	}
}

/// \private
std::ostream& operator<<(std::ostream& _out, const PubKey_read_field& _value) {
	_value.write(_out);
	return _out;
}

/// \private
std::istream& operator>>(std::istream& _in, PubKey_read_field& _value) {
	_value.read(_in);
	return _in;
}

const vnx::TypeCode* PubKey_read_field::static_get_type_code() {
	const vnx::TypeCode* type_code = vnx::get_type_code(VNX_TYPE_HASH);
	if(!type_code) {
		type_code = vnx::register_type_code(static_create_type_code());
	}
	return type_code;
}

std::shared_ptr<vnx::TypeCode> PubKey_read_field::static_create_type_code() {
	auto type_code = std::make_shared<vnx::TypeCode>();
	type_code->name = "mmx.contract.PubKey.read_field";
	type_code->type_hash = vnx::Hash64(0x8178b9776d46e140ull);
	type_code->code_hash = vnx::Hash64(0xa291725d0a1505deull);
	type_code->is_native = true;
	type_code->is_class = true;
	type_code->is_method = true;
	type_code->native_size = sizeof(::mmx::contract::PubKey_read_field);
	type_code->create_value = []() -> std::shared_ptr<vnx::Value> { return std::make_shared<PubKey_read_field>(); };
	type_code->is_const = true;
	type_code->return_type = ::mmx::contract::PubKey_read_field_return::static_get_type_code();
	type_code->fields.resize(1);
	{
		auto& field = type_code->fields[0];
		field.is_extended = true;
		field.name = "name";
		field.code = {32};
	}
	type_code->build();
	return type_code;
}


} // namespace mmx
} // namespace contract


namespace vnx {

void read(TypeInput& in, ::mmx::contract::PubKey_read_field& value, const TypeCode* type_code, const uint16_t* code) {
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
	in.read(type_code->total_field_size);
	if(type_code->is_matched) {
	}
	for(const auto* _field : type_code->ext_fields) {
		switch(_field->native_index) {
			case 0: vnx::read(in, value.name, type_code, _field->code.data()); break;
			default: vnx::skip(in, type_code, _field->code.data());
		}
	}
}

void write(TypeOutput& out, const ::mmx::contract::PubKey_read_field& value, const TypeCode* type_code, const uint16_t* code) {
	if(code && code[0] == CODE_OBJECT) {
		vnx::write(out, value.to_object(), nullptr, code);
		return;
	}
	if(!type_code || (code && code[0] == CODE_ANY)) {
		type_code = mmx::contract::vnx_native_type_code_PubKey_read_field;
		out.write_type_code(type_code);
		vnx::write_class_header<::mmx::contract::PubKey_read_field>(out);
	}
	else if(code && code[0] == CODE_STRUCT) {
		type_code = type_code->depends[code[1]];
	}
	vnx::write(out, value.name, type_code, type_code->fields[0].code.data());
}

void read(std::istream& in, ::mmx::contract::PubKey_read_field& value) {
	value.read(in);
}

void write(std::ostream& out, const ::mmx::contract::PubKey_read_field& value) {
	value.write(out);
}

void accept(Visitor& visitor, const ::mmx::contract::PubKey_read_field& value) {
	value.accept(visitor);
}

} // vnx

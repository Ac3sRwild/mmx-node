
// AUTO GENERATED by vnxcppcodegen

#include <mmx/package.hxx>
#include <mmx/balance_t.hxx>

#include <vnx/vnx.h>


namespace mmx {


const vnx::Hash64 balance_t::VNX_TYPE_HASH(0x613173c7e5ce65b4ull);
const vnx::Hash64 balance_t::VNX_CODE_HASH(0xb4bf28f806e74c8cull);

vnx::Hash64 balance_t::get_type_hash() const {
	return VNX_TYPE_HASH;
}

std::string balance_t::get_type_name() const {
	return "mmx.balance_t";
}

const vnx::TypeCode* balance_t::get_type_code() const {
	return mmx::vnx_native_type_code_balance_t;
}

std::shared_ptr<balance_t> balance_t::create() {
	return std::make_shared<balance_t>();
}

std::shared_ptr<balance_t> balance_t::clone() const {
	return std::make_shared<balance_t>(*this);
}

void balance_t::read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) {
	vnx::read(_in, *this, _type_code, _code);
}

void balance_t::write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const {
	vnx::write(_out, *this, _type_code, _code);
}

void balance_t::accept(vnx::Visitor& _visitor) const {
	const vnx::TypeCode* _type_code = mmx::vnx_native_type_code_balance_t;
	_visitor.type_begin(*_type_code);
	_visitor.type_field(_type_code->fields[0], 0); vnx::accept(_visitor, spendable);
	_visitor.type_field(_type_code->fields[1], 1); vnx::accept(_visitor, reserved);
	_visitor.type_field(_type_code->fields[2], 2); vnx::accept(_visitor, total);
	_visitor.type_end(*_type_code);
}

void balance_t::write(std::ostream& _out) const {
	_out << "{";
	_out << "\"spendable\": "; vnx::write(_out, spendable);
	_out << ", \"reserved\": "; vnx::write(_out, reserved);
	_out << ", \"total\": "; vnx::write(_out, total);
	_out << "}";
}

void balance_t::read(std::istream& _in) {
	if(auto _json = vnx::read_json(_in)) {
		from_object(_json->to_object());
	}
}

vnx::Object balance_t::to_object() const {
	vnx::Object _object;
	_object["__type"] = "mmx.balance_t";
	_object["spendable"] = spendable;
	_object["reserved"] = reserved;
	_object["total"] = total;
	return _object;
}

void balance_t::from_object(const vnx::Object& _object) {
	for(const auto& _entry : _object.field) {
		if(_entry.first == "reserved") {
			_entry.second.to(reserved);
		} else if(_entry.first == "spendable") {
			_entry.second.to(spendable);
		} else if(_entry.first == "total") {
			_entry.second.to(total);
		}
	}
}

vnx::Variant balance_t::get_field(const std::string& _name) const {
	if(_name == "spendable") {
		return vnx::Variant(spendable);
	}
	if(_name == "reserved") {
		return vnx::Variant(reserved);
	}
	if(_name == "total") {
		return vnx::Variant(total);
	}
	return vnx::Variant();
}

void balance_t::set_field(const std::string& _name, const vnx::Variant& _value) {
	if(_name == "spendable") {
		_value.to(spendable);
	} else if(_name == "reserved") {
		_value.to(reserved);
	} else if(_name == "total") {
		_value.to(total);
	}
}

/// \private
std::ostream& operator<<(std::ostream& _out, const balance_t& _value) {
	_value.write(_out);
	return _out;
}

/// \private
std::istream& operator>>(std::istream& _in, balance_t& _value) {
	_value.read(_in);
	return _in;
}

const vnx::TypeCode* balance_t::static_get_type_code() {
	const vnx::TypeCode* type_code = vnx::get_type_code(VNX_TYPE_HASH);
	if(!type_code) {
		type_code = vnx::register_type_code(static_create_type_code());
	}
	return type_code;
}

std::shared_ptr<vnx::TypeCode> balance_t::static_create_type_code() {
	auto type_code = std::make_shared<vnx::TypeCode>();
	type_code->name = "mmx.balance_t";
	type_code->type_hash = vnx::Hash64(0x613173c7e5ce65b4ull);
	type_code->code_hash = vnx::Hash64(0xb4bf28f806e74c8cull);
	type_code->is_native = true;
	type_code->native_size = sizeof(::mmx::balance_t);
	type_code->create_value = []() -> std::shared_ptr<vnx::Value> { return std::make_shared<vnx::Struct<balance_t>>(); };
	type_code->fields.resize(3);
	{
		auto& field = type_code->fields[0];
		field.data_size = 8;
		field.name = "spendable";
		field.code = {4};
	}
	{
		auto& field = type_code->fields[1];
		field.data_size = 8;
		field.name = "reserved";
		field.code = {4};
	}
	{
		auto& field = type_code->fields[2];
		field.data_size = 8;
		field.name = "total";
		field.code = {4};
	}
	type_code->build();
	return type_code;
}


} // namespace mmx


namespace vnx {

void read(TypeInput& in, ::mmx::balance_t& value, const TypeCode* type_code, const uint16_t* code) {
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
			vnx::read_value(_buf + _field->offset, value.spendable, _field->code.data());
		}
		if(const auto* const _field = type_code->field_map[1]) {
			vnx::read_value(_buf + _field->offset, value.reserved, _field->code.data());
		}
		if(const auto* const _field = type_code->field_map[2]) {
			vnx::read_value(_buf + _field->offset, value.total, _field->code.data());
		}
	}
	for(const auto* _field : type_code->ext_fields) {
		switch(_field->native_index) {
			default: vnx::skip(in, type_code, _field->code.data());
		}
	}
}

void write(TypeOutput& out, const ::mmx::balance_t& value, const TypeCode* type_code, const uint16_t* code) {
	if(code && code[0] == CODE_OBJECT) {
		vnx::write(out, value.to_object(), nullptr, code);
		return;
	}
	if(!type_code || (code && code[0] == CODE_ANY)) {
		type_code = mmx::vnx_native_type_code_balance_t;
		out.write_type_code(type_code);
		vnx::write_class_header<::mmx::balance_t>(out);
	}
	else if(code && code[0] == CODE_STRUCT) {
		type_code = type_code->depends[code[1]];
	}
	char* const _buf = out.write(24);
	vnx::write_value(_buf + 0, value.spendable);
	vnx::write_value(_buf + 8, value.reserved);
	vnx::write_value(_buf + 16, value.total);
}

void read(std::istream& in, ::mmx::balance_t& value) {
	value.read(in);
}

void write(std::ostream& out, const ::mmx::balance_t& value) {
	value.write(out);
}

void accept(Visitor& visitor, const ::mmx::balance_t& value) {
	value.accept(visitor);
}

bool is_equivalent<::mmx::balance_t>::operator()(const uint16_t* code, const TypeCode* type_code) {
	if(code[0] != CODE_STRUCT || !type_code) {
		return false;
	}
	type_code = type_code->depends[code[1]];
	return type_code->type_hash == ::mmx::balance_t::VNX_TYPE_HASH && type_code->is_equivalent;
}

} // vnx

// AUTO GENERATED by vnxcppcodegen

#include <mmx/exchange/package.hxx>
#include <mmx/exchange/Client_execute.hxx>
#include <mmx/exchange/Client_execute_return.hxx>
#include <mmx/exchange/matched_order_t.hxx>
#include <vnx/Value.h>

#include <vnx/vnx.h>


namespace mmx {
namespace exchange {


const vnx::Hash64 Client_execute::VNX_TYPE_HASH(0xa683dfd1653658acull);
const vnx::Hash64 Client_execute::VNX_CODE_HASH(0xca2e80d962a7789cull);

vnx::Hash64 Client_execute::get_type_hash() const {
	return VNX_TYPE_HASH;
}

std::string Client_execute::get_type_name() const {
	return "mmx.exchange.Client.execute";
}

const vnx::TypeCode* Client_execute::get_type_code() const {
	return mmx::exchange::vnx_native_type_code_Client_execute;
}

std::shared_ptr<Client_execute> Client_execute::create() {
	return std::make_shared<Client_execute>();
}

std::shared_ptr<vnx::Value> Client_execute::clone() const {
	return std::make_shared<Client_execute>(*this);
}

void Client_execute::read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) {
	vnx::read(_in, *this, _type_code, _code);
}

void Client_execute::write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const {
	vnx::write(_out, *this, _type_code, _code);
}

void Client_execute::accept(vnx::Visitor& _visitor) const {
	const vnx::TypeCode* _type_code = mmx::exchange::vnx_native_type_code_Client_execute;
	_visitor.type_begin(*_type_code);
	_visitor.type_field(_type_code->fields[0], 0); vnx::accept(_visitor, server);
	_visitor.type_field(_type_code->fields[1], 1); vnx::accept(_visitor, wallet);
	_visitor.type_field(_type_code->fields[2], 2); vnx::accept(_visitor, order);
	_visitor.type_end(*_type_code);
}

void Client_execute::write(std::ostream& _out) const {
	_out << "{\"__type\": \"mmx.exchange.Client.execute\"";
	_out << ", \"server\": "; vnx::write(_out, server);
	_out << ", \"wallet\": "; vnx::write(_out, wallet);
	_out << ", \"order\": "; vnx::write(_out, order);
	_out << "}";
}

void Client_execute::read(std::istream& _in) {
	if(auto _json = vnx::read_json(_in)) {
		from_object(_json->to_object());
	}
}

vnx::Object Client_execute::to_object() const {
	vnx::Object _object;
	_object["__type"] = "mmx.exchange.Client.execute";
	_object["server"] = server;
	_object["wallet"] = wallet;
	_object["order"] = order;
	return _object;
}

void Client_execute::from_object(const vnx::Object& _object) {
	for(const auto& _entry : _object.field) {
		if(_entry.first == "order") {
			_entry.second.to(order);
		} else if(_entry.first == "server") {
			_entry.second.to(server);
		} else if(_entry.first == "wallet") {
			_entry.second.to(wallet);
		}
	}
}

vnx::Variant Client_execute::get_field(const std::string& _name) const {
	if(_name == "server") {
		return vnx::Variant(server);
	}
	if(_name == "wallet") {
		return vnx::Variant(wallet);
	}
	if(_name == "order") {
		return vnx::Variant(order);
	}
	return vnx::Variant();
}

void Client_execute::set_field(const std::string& _name, const vnx::Variant& _value) {
	if(_name == "server") {
		_value.to(server);
	} else if(_name == "wallet") {
		_value.to(wallet);
	} else if(_name == "order") {
		_value.to(order);
	}
}

/// \private
std::ostream& operator<<(std::ostream& _out, const Client_execute& _value) {
	_value.write(_out);
	return _out;
}

/// \private
std::istream& operator>>(std::istream& _in, Client_execute& _value) {
	_value.read(_in);
	return _in;
}

const vnx::TypeCode* Client_execute::static_get_type_code() {
	const vnx::TypeCode* type_code = vnx::get_type_code(VNX_TYPE_HASH);
	if(!type_code) {
		type_code = vnx::register_type_code(static_create_type_code());
	}
	return type_code;
}

std::shared_ptr<vnx::TypeCode> Client_execute::static_create_type_code() {
	auto type_code = std::make_shared<vnx::TypeCode>();
	type_code->name = "mmx.exchange.Client.execute";
	type_code->type_hash = vnx::Hash64(0xa683dfd1653658acull);
	type_code->code_hash = vnx::Hash64(0xca2e80d962a7789cull);
	type_code->is_native = true;
	type_code->is_class = true;
	type_code->is_method = true;
	type_code->native_size = sizeof(::mmx::exchange::Client_execute);
	type_code->create_value = []() -> std::shared_ptr<vnx::Value> { return std::make_shared<Client_execute>(); };
	type_code->depends.resize(1);
	type_code->depends[0] = ::mmx::exchange::matched_order_t::static_get_type_code();
	type_code->is_const = true;
	type_code->is_async = true;
	type_code->return_type = ::mmx::exchange::Client_execute_return::static_get_type_code();
	type_code->fields.resize(3);
	{
		auto& field = type_code->fields[0];
		field.is_extended = true;
		field.name = "server";
		field.code = {32};
	}
	{
		auto& field = type_code->fields[1];
		field.data_size = 4;
		field.name = "wallet";
		field.code = {3};
	}
	{
		auto& field = type_code->fields[2];
		field.is_extended = true;
		field.name = "order";
		field.code = {19, 0};
	}
	type_code->permission = "mmx.permission_e.SPENDING";
	type_code->build();
	return type_code;
}


} // namespace mmx
} // namespace exchange


namespace vnx {

void read(TypeInput& in, ::mmx::exchange::Client_execute& value, const TypeCode* type_code, const uint16_t* code) {
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
		if(const auto* const _field = type_code->field_map[1]) {
			vnx::read_value(_buf + _field->offset, value.wallet, _field->code.data());
		}
	}
	for(const auto* _field : type_code->ext_fields) {
		switch(_field->native_index) {
			case 0: vnx::read(in, value.server, type_code, _field->code.data()); break;
			case 2: vnx::read(in, value.order, type_code, _field->code.data()); break;
			default: vnx::skip(in, type_code, _field->code.data());
		}
	}
}

void write(TypeOutput& out, const ::mmx::exchange::Client_execute& value, const TypeCode* type_code, const uint16_t* code) {
	if(code && code[0] == CODE_OBJECT) {
		vnx::write(out, value.to_object(), nullptr, code);
		return;
	}
	if(!type_code || (code && code[0] == CODE_ANY)) {
		type_code = mmx::exchange::vnx_native_type_code_Client_execute;
		out.write_type_code(type_code);
		vnx::write_class_header<::mmx::exchange::Client_execute>(out);
	}
	else if(code && code[0] == CODE_STRUCT) {
		type_code = type_code->depends[code[1]];
	}
	char* const _buf = out.write(4);
	vnx::write_value(_buf + 0, value.wallet);
	vnx::write(out, value.server, type_code, type_code->fields[0].code.data());
	vnx::write(out, value.order, type_code, type_code->fields[2].code.data());
}

void read(std::istream& in, ::mmx::exchange::Client_execute& value) {
	value.read(in);
}

void write(std::ostream& out, const ::mmx::exchange::Client_execute& value) {
	value.write(out);
}

void accept(Visitor& visitor, const ::mmx::exchange::Client_execute& value) {
	value.accept(visitor);
}

} // vnx

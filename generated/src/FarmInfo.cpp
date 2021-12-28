
// AUTO GENERATED by vnxcppcodegen

#include <mmx/package.hxx>
#include <mmx/FarmInfo.hxx>
#include <vnx/Value.h>

#include <vnx/vnx.h>


namespace mmx {


const vnx::Hash64 FarmInfo::VNX_TYPE_HASH(0xa2701372b9137f0eull);
const vnx::Hash64 FarmInfo::VNX_CODE_HASH(0x53b09e8f811c1d07ull);

vnx::Hash64 FarmInfo::get_type_hash() const {
	return VNX_TYPE_HASH;
}

std::string FarmInfo::get_type_name() const {
	return "mmx.FarmInfo";
}

const vnx::TypeCode* FarmInfo::get_type_code() const {
	return mmx::vnx_native_type_code_FarmInfo;
}

std::shared_ptr<FarmInfo> FarmInfo::create() {
	return std::make_shared<FarmInfo>();
}

std::shared_ptr<vnx::Value> FarmInfo::clone() const {
	return std::make_shared<FarmInfo>(*this);
}

void FarmInfo::read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) {
	vnx::read(_in, *this, _type_code, _code);
}

void FarmInfo::write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const {
	vnx::write(_out, *this, _type_code, _code);
}

void FarmInfo::accept(vnx::Visitor& _visitor) const {
	const vnx::TypeCode* _type_code = mmx::vnx_native_type_code_FarmInfo;
	_visitor.type_begin(*_type_code);
	_visitor.type_field(_type_code->fields[0], 0); vnx::accept(_visitor, plot_dirs);
	_visitor.type_field(_type_code->fields[1], 1); vnx::accept(_visitor, plot_count);
	_visitor.type_field(_type_code->fields[2], 2); vnx::accept(_visitor, total_bytes);
	_visitor.type_end(*_type_code);
}

void FarmInfo::write(std::ostream& _out) const {
	_out << "{\"__type\": \"mmx.FarmInfo\"";
	_out << ", \"plot_dirs\": "; vnx::write(_out, plot_dirs);
	_out << ", \"plot_count\": "; vnx::write(_out, plot_count);
	_out << ", \"total_bytes\": "; vnx::write(_out, total_bytes);
	_out << "}";
}

void FarmInfo::read(std::istream& _in) {
	if(auto _json = vnx::read_json(_in)) {
		from_object(_json->to_object());
	}
}

vnx::Object FarmInfo::to_object() const {
	vnx::Object _object;
	_object["__type"] = "mmx.FarmInfo";
	_object["plot_dirs"] = plot_dirs;
	_object["plot_count"] = plot_count;
	_object["total_bytes"] = total_bytes;
	return _object;
}

void FarmInfo::from_object(const vnx::Object& _object) {
	for(const auto& _entry : _object.field) {
		if(_entry.first == "plot_count") {
			_entry.second.to(plot_count);
		} else if(_entry.first == "plot_dirs") {
			_entry.second.to(plot_dirs);
		} else if(_entry.first == "total_bytes") {
			_entry.second.to(total_bytes);
		}
	}
}

vnx::Variant FarmInfo::get_field(const std::string& _name) const {
	if(_name == "plot_dirs") {
		return vnx::Variant(plot_dirs);
	}
	if(_name == "plot_count") {
		return vnx::Variant(plot_count);
	}
	if(_name == "total_bytes") {
		return vnx::Variant(total_bytes);
	}
	return vnx::Variant();
}

void FarmInfo::set_field(const std::string& _name, const vnx::Variant& _value) {
	if(_name == "plot_dirs") {
		_value.to(plot_dirs);
	} else if(_name == "plot_count") {
		_value.to(plot_count);
	} else if(_name == "total_bytes") {
		_value.to(total_bytes);
	} else {
		throw std::logic_error("no such field: '" + _name + "'");
	}
}

/// \private
std::ostream& operator<<(std::ostream& _out, const FarmInfo& _value) {
	_value.write(_out);
	return _out;
}

/// \private
std::istream& operator>>(std::istream& _in, FarmInfo& _value) {
	_value.read(_in);
	return _in;
}

const vnx::TypeCode* FarmInfo::static_get_type_code() {
	const vnx::TypeCode* type_code = vnx::get_type_code(VNX_TYPE_HASH);
	if(!type_code) {
		type_code = vnx::register_type_code(static_create_type_code());
	}
	return type_code;
}

std::shared_ptr<vnx::TypeCode> FarmInfo::static_create_type_code() {
	auto type_code = std::make_shared<vnx::TypeCode>();
	type_code->name = "mmx.FarmInfo";
	type_code->type_hash = vnx::Hash64(0xa2701372b9137f0eull);
	type_code->code_hash = vnx::Hash64(0x53b09e8f811c1d07ull);
	type_code->is_native = true;
	type_code->is_class = true;
	type_code->native_size = sizeof(::mmx::FarmInfo);
	type_code->create_value = []() -> std::shared_ptr<vnx::Value> { return std::make_shared<FarmInfo>(); };
	type_code->fields.resize(3);
	{
		auto& field = type_code->fields[0];
		field.is_extended = true;
		field.name = "plot_dirs";
		field.code = {12, 32};
	}
	{
		auto& field = type_code->fields[1];
		field.is_extended = true;
		field.name = "plot_count";
		field.code = {13, 3, 1, 3};
	}
	{
		auto& field = type_code->fields[2];
		field.data_size = 8;
		field.name = "total_bytes";
		field.code = {4};
	}
	type_code->build();
	return type_code;
}


} // namespace mmx


namespace vnx {

void read(TypeInput& in, ::mmx::FarmInfo& value, const TypeCode* type_code, const uint16_t* code) {
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
		if(const auto* const _field = type_code->field_map[2]) {
			vnx::read_value(_buf + _field->offset, value.total_bytes, _field->code.data());
		}
	}
	for(const auto* _field : type_code->ext_fields) {
		switch(_field->native_index) {
			case 0: vnx::read(in, value.plot_dirs, type_code, _field->code.data()); break;
			case 1: vnx::read(in, value.plot_count, type_code, _field->code.data()); break;
			default: vnx::skip(in, type_code, _field->code.data());
		}
	}
}

void write(TypeOutput& out, const ::mmx::FarmInfo& value, const TypeCode* type_code, const uint16_t* code) {
	if(code && code[0] == CODE_OBJECT) {
		vnx::write(out, value.to_object(), nullptr, code);
		return;
	}
	if(!type_code || (code && code[0] == CODE_ANY)) {
		type_code = mmx::vnx_native_type_code_FarmInfo;
		out.write_type_code(type_code);
		vnx::write_class_header<::mmx::FarmInfo>(out);
	}
	else if(code && code[0] == CODE_STRUCT) {
		type_code = type_code->depends[code[1]];
	}
	char* const _buf = out.write(8);
	vnx::write_value(_buf + 0, value.total_bytes);
	vnx::write(out, value.plot_dirs, type_code, type_code->fields[0].code.data());
	vnx::write(out, value.plot_count, type_code, type_code->fields[1].code.data());
}

void read(std::istream& in, ::mmx::FarmInfo& value) {
	value.read(in);
}

void write(std::ostream& out, const ::mmx::FarmInfo& value) {
	value.write(out);
}

void accept(Visitor& visitor, const ::mmx::FarmInfo& value) {
	value.accept(visitor);
}

} // vnx

// AUTO GENERATED by vnxcppcodegen

#ifndef INCLUDE_mmx_Transaction_get_tx_index_HXX_
#define INCLUDE_mmx_Transaction_get_tx_index_HXX_

#include <mmx/package.hxx>
#include <mmx/ChainParams.hxx>
#include <vnx/Value.h>


namespace mmx {

class MMX_EXPORT Transaction_get_tx_index : public ::vnx::Value {
public:
	
	std::shared_ptr<const ::mmx::ChainParams> params;
	uint32_t height = 0;
	int64_t file_offset = 0;
	
	typedef ::vnx::Value Super;
	
	static const vnx::Hash64 VNX_TYPE_HASH;
	static const vnx::Hash64 VNX_CODE_HASH;
	
	static constexpr uint64_t VNX_TYPE_ID = 0x74c1207b72c7068eull;
	
	Transaction_get_tx_index() {}
	
	vnx::Hash64 get_type_hash() const override;
	std::string get_type_name() const override;
	const vnx::TypeCode* get_type_code() const override;
	
	static std::shared_ptr<Transaction_get_tx_index> create();
	std::shared_ptr<vnx::Value> clone() const override;
	
	void read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) override;
	void write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const override;
	
	void read(std::istream& _in) override;
	void write(std::ostream& _out) const override;
	
	template<typename T>
	void accept_generic(T& _visitor) const;
	void accept(vnx::Visitor& _visitor) const override;
	
	vnx::Object to_object() const override;
	void from_object(const vnx::Object& object) override;
	
	vnx::Variant get_field(const std::string& name) const override;
	void set_field(const std::string& name, const vnx::Variant& value) override;
	
	friend std::ostream& operator<<(std::ostream& _out, const Transaction_get_tx_index& _value);
	friend std::istream& operator>>(std::istream& _in, Transaction_get_tx_index& _value);
	
	static const vnx::TypeCode* static_get_type_code();
	static std::shared_ptr<vnx::TypeCode> static_create_type_code();
	
};

template<typename T>
void Transaction_get_tx_index::accept_generic(T& _visitor) const {
	_visitor.template type_begin<Transaction_get_tx_index>(3);
	_visitor.type_field("params", 0); _visitor.accept(params);
	_visitor.type_field("height", 1); _visitor.accept(height);
	_visitor.type_field("file_offset", 2); _visitor.accept(file_offset);
	_visitor.template type_end<Transaction_get_tx_index>(3);
}


} // namespace mmx


namespace vnx {

} // vnx

#endif // INCLUDE_mmx_Transaction_get_tx_index_HXX_

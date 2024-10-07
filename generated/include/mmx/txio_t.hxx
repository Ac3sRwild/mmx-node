
// AUTO GENERATED by vnxcppcodegen

#ifndef INCLUDE_mmx_txio_t_HXX_
#define INCLUDE_mmx_txio_t_HXX_

#include <vnx/Type.h>
#include <mmx/package.hxx>
#include <mmx/ChainParams.hxx>
#include <mmx/addr_t.hpp>
#include <mmx/uint128.hpp>


namespace mmx {

struct MMX_EXPORT txio_t : vnx::struct_t {
	
	static const uint32_t MAX_MEMO_SIZE = 64;
	
	::mmx::addr_t address;
	::mmx::addr_t contract;
	::mmx::uint128 amount;
	vnx::optional<std::string> memo;
	
	static const vnx::Hash64 VNX_TYPE_HASH;
	static const vnx::Hash64 VNX_CODE_HASH;
	
	static constexpr uint64_t VNX_TYPE_ID = 0x32adb93b85c82cf4ull;
	
	txio_t() {}
	
	vnx::Hash64 get_type_hash() const;
	std::string get_type_name() const;
	const vnx::TypeCode* get_type_code() const;
	
	uint64_t calc_cost(std::shared_ptr<const ::mmx::ChainParams> params = nullptr) const;
	
	static std::shared_ptr<txio_t> create();
	std::shared_ptr<txio_t> clone() const;
	
	void read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code);
	void write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const;
	
	void read(std::istream& _in);
	void write(std::ostream& _out) const;
	
	template<typename T>
	void accept_generic(T& _visitor) const;
	void accept(vnx::Visitor& _visitor) const;
	
	vnx::Object to_object() const;
	void from_object(const vnx::Object& object);
	
	vnx::Variant get_field(const std::string& name) const;
	void set_field(const std::string& name, const vnx::Variant& value);
	
	friend std::ostream& operator<<(std::ostream& _out, const txio_t& _value);
	friend std::istream& operator>>(std::istream& _in, txio_t& _value);
	
	static const vnx::TypeCode* static_get_type_code();
	static std::shared_ptr<vnx::TypeCode> static_create_type_code();
	
};

template<typename T>
void txio_t::accept_generic(T& _visitor) const {
	_visitor.template type_begin<txio_t>(4);
	_visitor.type_field("address", 0); _visitor.accept(address);
	_visitor.type_field("contract", 1); _visitor.accept(contract);
	_visitor.type_field("amount", 2); _visitor.accept(amount);
	_visitor.type_field("memo", 3); _visitor.accept(memo);
	_visitor.template type_end<txio_t>(4);
}


} // namespace mmx


namespace vnx {

template<>
struct is_equivalent<::mmx::txio_t> {
	bool operator()(const uint16_t* code, const TypeCode* type_code);
};

} // vnx

#endif // INCLUDE_mmx_txio_t_HXX_

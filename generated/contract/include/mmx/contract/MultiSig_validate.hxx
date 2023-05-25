
// AUTO GENERATED by vnxcppcodegen

#ifndef INCLUDE_mmx_contract_MultiSig_validate_HXX_
#define INCLUDE_mmx_contract_MultiSig_validate_HXX_

#include <mmx/contract/package.hxx>
#include <mmx/Solution.hxx>
#include <mmx/hash_t.hpp>
#include <vnx/Value.h>


namespace mmx {
namespace contract {

class MMX_CONTRACT_EXPORT MultiSig_validate : public ::vnx::Value {
public:
	
	std::shared_ptr<const ::mmx::Solution> solution;
	::mmx::hash_t txid;
	
	typedef ::vnx::Value Super;
	
	static const vnx::Hash64 VNX_TYPE_HASH;
	static const vnx::Hash64 VNX_CODE_HASH;
	
	static constexpr uint64_t VNX_TYPE_ID = 0x40855ac13f61e392ull;
	
	MultiSig_validate() {}
	
	vnx::Hash64 get_type_hash() const override;
	std::string get_type_name() const override;
	const vnx::TypeCode* get_type_code() const override;
	
	static std::shared_ptr<MultiSig_validate> create();
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
	
	friend std::ostream& operator<<(std::ostream& _out, const MultiSig_validate& _value);
	friend std::istream& operator>>(std::istream& _in, MultiSig_validate& _value);
	
	static const vnx::TypeCode* static_get_type_code();
	static std::shared_ptr<vnx::TypeCode> static_create_type_code();
	
};

template<typename T>
void MultiSig_validate::accept_generic(T& _visitor) const {
	_visitor.template type_begin<MultiSig_validate>(2);
	_visitor.type_field("solution", 0); _visitor.accept(solution);
	_visitor.type_field("txid", 1); _visitor.accept(txid);
	_visitor.template type_end<MultiSig_validate>(2);
}


} // namespace mmx
} // namespace contract


namespace vnx {

} // vnx

#endif // INCLUDE_mmx_contract_MultiSig_validate_HXX_

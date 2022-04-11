
// AUTO GENERATED by vnxcppcodegen

#ifndef INCLUDE_mmx_solution_PubKey_HXX_
#define INCLUDE_mmx_solution_PubKey_HXX_

#include <mmx/solution/package.hxx>
#include <mmx/ChainParams.hxx>
#include <mmx/Solution.hxx>
#include <mmx/pubkey_t.hpp>
#include <mmx/signature_t.hpp>


namespace mmx {
namespace solution {

class MMX_SOLUTION_EXPORT PubKey : public ::mmx::Solution {
public:
	
	::mmx::pubkey_t pubkey;
	::mmx::signature_t signature;
	
	typedef ::mmx::Solution Super;
	
	static const vnx::Hash64 VNX_TYPE_HASH;
	static const vnx::Hash64 VNX_CODE_HASH;
	
	static constexpr uint64_t VNX_TYPE_ID = 0xe47af6fcacfcefa5ull;
	
	PubKey() {}
	
	vnx::Hash64 get_type_hash() const override;
	std::string get_type_name() const override;
	const vnx::TypeCode* get_type_code() const override;
	
	virtual vnx::bool_t is_valid() const override;
	virtual uint64_t calc_cost(std::shared_ptr<const ::mmx::ChainParams> params = nullptr) const override;
	
	static std::shared_ptr<PubKey> create();
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
	
	friend std::ostream& operator<<(std::ostream& _out, const PubKey& _value);
	friend std::istream& operator>>(std::istream& _in, PubKey& _value);
	
	static const vnx::TypeCode* static_get_type_code();
	static std::shared_ptr<vnx::TypeCode> static_create_type_code();
	
protected:
	std::shared_ptr<vnx::Value> vnx_call_switch(std::shared_ptr<const vnx::Value> _method) override;
	
};

template<typename T>
void PubKey::accept_generic(T& _visitor) const {
	_visitor.template type_begin<PubKey>(3);
	_visitor.type_field("version", 0); _visitor.accept(version);
	_visitor.type_field("pubkey", 1); _visitor.accept(pubkey);
	_visitor.type_field("signature", 2); _visitor.accept(signature);
	_visitor.template type_end<PubKey>(3);
}


} // namespace mmx
} // namespace solution


namespace vnx {

} // vnx

#endif // INCLUDE_mmx_solution_PubKey_HXX_

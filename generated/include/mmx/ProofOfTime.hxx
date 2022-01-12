
// AUTO GENERATED by vnxcppcodegen

#ifndef INCLUDE_mmx_ProofOfTime_HXX_
#define INCLUDE_mmx_ProofOfTime_HXX_

#include <mmx/package.hxx>
#include <mmx/addr_t.hpp>
#include <mmx/hash_t.hpp>
#include <mmx/pubkey_t.hpp>
#include <mmx/signature_t.hpp>
#include <mmx/time_segment_t.hxx>
#include <vnx/Value.h>


namespace mmx {

class ProofOfTime : public ::vnx::Value {
public:
	
	uint32_t height = 0;
	uint64_t start = 0;
	std::array<::mmx::hash_t, 2> input = {};
	std::array<vnx::optional<::mmx::hash_t>, 2> infuse = {};
	std::vector<::mmx::time_segment_t> segments;
	vnx::optional<::mmx::hash_t> timelord_proof;
	vnx::optional<::mmx::addr_t> timelord_reward;
	::mmx::pubkey_t timelord_key;
	::mmx::signature_t timelord_sig;
	
	typedef ::vnx::Value Super;
	
	static const vnx::Hash64 VNX_TYPE_HASH;
	static const vnx::Hash64 VNX_CODE_HASH;
	
	static constexpr uint64_t VNX_TYPE_ID = 0xa84a63942b8e5c6aull;
	
	ProofOfTime() {}
	
	vnx::Hash64 get_type_hash() const override;
	std::string get_type_name() const override;
	const vnx::TypeCode* get_type_code() const override;
	
	virtual ::mmx::hash_t calc_hash() const;
	virtual ::mmx::hash_t get_output(const uint32_t& chain = 0) const;
	virtual uint64_t get_num_iters() const;
	virtual uint64_t get_vdf_iters() const;
	virtual std::shared_ptr<const ::mmx::ProofOfTime> compressed() const;
	
	static std::shared_ptr<ProofOfTime> create();
	std::shared_ptr<vnx::Value> clone() const override;
	
	void read(vnx::TypeInput& _in, const vnx::TypeCode* _type_code, const uint16_t* _code) override;
	void write(vnx::TypeOutput& _out, const vnx::TypeCode* _type_code, const uint16_t* _code) const override;
	
	void read(std::istream& _in) override;
	void write(std::ostream& _out) const override;
	
	void accept(vnx::Visitor& _visitor) const override;
	
	vnx::Object to_object() const override;
	void from_object(const vnx::Object& object) override;
	
	vnx::Variant get_field(const std::string& name) const override;
	void set_field(const std::string& name, const vnx::Variant& value) override;
	
	friend std::ostream& operator<<(std::ostream& _out, const ProofOfTime& _value);
	friend std::istream& operator>>(std::istream& _in, ProofOfTime& _value);
	
	static const vnx::TypeCode* static_get_type_code();
	static std::shared_ptr<vnx::TypeCode> static_create_type_code();
	
};


} // namespace mmx


namespace vnx {

} // vnx

#endif // INCLUDE_mmx_ProofOfTime_HXX_

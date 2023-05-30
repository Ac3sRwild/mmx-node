
// AUTO GENERATED by vnxcppcodegen

#ifndef INCLUDE_mmx_BlockHeader_HXX_
#define INCLUDE_mmx_BlockHeader_HXX_

#include <mmx/package.hxx>
#include <mmx/ProofOfSpace.hxx>
#include <mmx/addr_t.hpp>
#include <mmx/block_index_t.hxx>
#include <mmx/bls_signature_t.hpp>
#include <mmx/hash_t.hpp>
#include <mmx/uint128.hpp>
#include <vnx/Value.h>


namespace mmx {

class MMX_EXPORT BlockHeader : public ::vnx::Value {
public:
	
	uint32_t version = 0;
	::mmx::hash_t hash;
	::mmx::hash_t prev;
	uint32_t height = 0;
	uint64_t nonce = 0;
	uint64_t time_diff = 0;
	uint64_t space_diff = 0;
	::mmx::uint128 weight;
	::mmx::uint128 total_weight;
	uint32_t netspace_ratio = 0;
	uint32_t average_txfee = 0;
	uint64_t vdf_iters = 0;
	std::array<::mmx::hash_t, 2> vdf_output = {};
	vnx::optional<::mmx::addr_t> vdf_reward_addr;
	std::shared_ptr<const ::mmx::ProofOfSpace> proof;
	uint64_t reward_amount = 0;
	vnx::optional<::mmx::addr_t> reward_addr;
	uint32_t static_cost = 0;
	uint32_t total_cost = 0;
	uint32_t tx_count = 0;
	uint32_t tx_fees = 0;
	::mmx::hash_t tx_hash;
	vnx::optional<::mmx::bls_signature_t> farmer_sig;
	::mmx::hash_t content_hash;
	
	typedef ::vnx::Value Super;
	
	static const vnx::Hash64 VNX_TYPE_HASH;
	static const vnx::Hash64 VNX_CODE_HASH;
	
	static constexpr uint64_t VNX_TYPE_ID = 0xcaae941a2fc712a6ull;
	
	BlockHeader() {}
	
	vnx::Hash64 get_type_hash() const override;
	std::string get_type_name() const override;
	const vnx::TypeCode* get_type_code() const override;
	
	virtual vnx::bool_t is_valid() const;
	virtual std::pair<::mmx::hash_t, ::mmx::hash_t> calc_hash() const;
	virtual void validate() const;
	virtual std::shared_ptr<const ::mmx::BlockHeader> get_header() const;
	virtual ::mmx::block_index_t get_block_index(const int64_t& file_offset = 0) const;
	
	static std::shared_ptr<BlockHeader> create();
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
	
	friend std::ostream& operator<<(std::ostream& _out, const BlockHeader& _value);
	friend std::istream& operator>>(std::istream& _in, BlockHeader& _value);
	
	static const vnx::TypeCode* static_get_type_code();
	static std::shared_ptr<vnx::TypeCode> static_create_type_code();
	
protected:
	std::shared_ptr<vnx::Value> vnx_call_switch(std::shared_ptr<const vnx::Value> _method) override;
	
};

template<typename T>
void BlockHeader::accept_generic(T& _visitor) const {
	_visitor.template type_begin<BlockHeader>(24);
	_visitor.type_field("version", 0); _visitor.accept(version);
	_visitor.type_field("hash", 1); _visitor.accept(hash);
	_visitor.type_field("prev", 2); _visitor.accept(prev);
	_visitor.type_field("height", 3); _visitor.accept(height);
	_visitor.type_field("nonce", 4); _visitor.accept(nonce);
	_visitor.type_field("time_diff", 5); _visitor.accept(time_diff);
	_visitor.type_field("space_diff", 6); _visitor.accept(space_diff);
	_visitor.type_field("weight", 7); _visitor.accept(weight);
	_visitor.type_field("total_weight", 8); _visitor.accept(total_weight);
	_visitor.type_field("netspace_ratio", 9); _visitor.accept(netspace_ratio);
	_visitor.type_field("average_txfee", 10); _visitor.accept(average_txfee);
	_visitor.type_field("vdf_iters", 11); _visitor.accept(vdf_iters);
	_visitor.type_field("vdf_output", 12); _visitor.accept(vdf_output);
	_visitor.type_field("vdf_reward_addr", 13); _visitor.accept(vdf_reward_addr);
	_visitor.type_field("proof", 14); _visitor.accept(proof);
	_visitor.type_field("reward_amount", 15); _visitor.accept(reward_amount);
	_visitor.type_field("reward_addr", 16); _visitor.accept(reward_addr);
	_visitor.type_field("static_cost", 17); _visitor.accept(static_cost);
	_visitor.type_field("total_cost", 18); _visitor.accept(total_cost);
	_visitor.type_field("tx_count", 19); _visitor.accept(tx_count);
	_visitor.type_field("tx_fees", 20); _visitor.accept(tx_fees);
	_visitor.type_field("tx_hash", 21); _visitor.accept(tx_hash);
	_visitor.type_field("farmer_sig", 22); _visitor.accept(farmer_sig);
	_visitor.type_field("content_hash", 23); _visitor.accept(content_hash);
	_visitor.template type_end<BlockHeader>(24);
}


} // namespace mmx


namespace vnx {

} // vnx

#endif // INCLUDE_mmx_BlockHeader_HXX_

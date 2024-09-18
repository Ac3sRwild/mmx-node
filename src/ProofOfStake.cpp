/*
 * ProofOfStake.cpp
 *
 *  Created on: Apr 13, 2022
 *      Author: mad
 */

#include <mmx/ProofOfStake.hxx>
#include <mmx/write_bytes.h>


namespace mmx {

hash_t ProofOfStake::calc_hash() const
{
	std::vector<uint8_t> buffer;
	vnx::VectorOutputStream stream(&buffer);
	vnx::OutputBuffer out(&stream);

	buffer.reserve(4 * 1024);

	write_bytes(out, get_type_hash());
	write_field(out, "score", 		score);
	write_field(out, "plot_id", 	plot_id);
	write_field(out, "farmer_key", 	farmer_key);
	out.flush();

	return hash_t(buffer);
}

void ProofOfStake::validate() const
{
	// nothing to do
}


} // mmx

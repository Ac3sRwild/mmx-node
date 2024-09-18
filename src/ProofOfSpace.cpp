/*
 * ProofOfSpace.cpp
 *
 *  Created on: Apr 13, 2022
 *      Author: mad
 */

#include <mmx/ProofOfSpace.hxx>
#include <mmx/write_bytes.h>


namespace mmx {

vnx::bool_t ProofOfSpace::is_valid() const
{
	return true;
}

mmx::hash_t ProofOfSpace::calc_hash() const
{
	throw std::logic_error("not implemented");
}

void ProofOfSpace::validate() const
{
	throw std::logic_error("not implemented");
}


} // mmx

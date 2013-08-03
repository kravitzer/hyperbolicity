/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_NODE_HASHER_H
#define DELTAHYPER_NODE_HASHER_H

#include "Node.h"

namespace dhtoolkit
{
	//functor for hashing node_ptr_t
	struct NodeHasher
	{
		std::size_t operator() (const node_ptr_t node) const
		{
			return std::hash<node_index_t>()(node->getIndex());
		}
	};
} // namespace dhtoolkit

#endif // DELTAHYPER_NODE_HASHER_H
/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_DEFS_H
#define DELTAHYPER_DEFS_H

#include <memory>
#include <vector>
#include <unordered_map>

namespace dhtoolkit
{
	//forward declarations
	class Graph;
	class Node;

	//definitions
	typedef size_t      									node_index_t;
	typedef std::shared_ptr<Node>							node_ptr_t;
	typedef std::vector<node_ptr_t>							node_ptr_collection_t;
	typedef std::weak_ptr<Node>								node_weak_ptr_t;
	typedef std::vector<node_weak_ptr_t>					node_weak_ptr_collection_t;

	typedef std::shared_ptr<Graph>							graph_ptr_t;
	typedef std::vector<graph_ptr_t>						graph_ptr_collection_t;

	typedef	int												distance_t;
	typedef std::unordered_map<node_index_t, distance_t>	distance_dict_t;

	//constants
	const distance_t										InfiniteDistance = -1;
}

#endif
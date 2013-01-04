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
	class Graph;
	class Node;
	class State;

	typedef unsigned int						node_index_t;
	typedef std::shared_ptr<Node>				node_ptr_t;
	typedef std::vector<node_ptr_t>				node_collection_t;

	typedef double								delta_t;
	const delta_t								InfiniteDelta = -1;
	typedef State								node_quad_t;
	typedef std::shared_ptr<Graph>				graph_ptr_t;

	typedef	int									distance_t;
	const distance_t							InfiniteDistance = -1;
	typedef std::unordered_map<node_index_t, 
							distance_t>			distance_dict_t;

}

#endif
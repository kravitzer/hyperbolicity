#ifndef DELTAHYPER_DEFS_H
#define DELTAHYPER_DEFS_H

#include <memory>
#include <vector>
#include <map>
#include <unordered_map>

namespace graphs
{
	class Graph;
	class Node;

	typedef unsigned int						node_index_t;
	typedef std::shared_ptr<Node>				node_ptr_t;
	typedef std::vector<node_ptr_t>				node_collection_t;

	typedef double								delta_t;
	const unsigned int							NodeQuadCount = 4;
	typedef node_ptr_t							node_quad_t[NodeQuadCount];

	typedef std::shared_ptr<Graph>				graph_ptr_t;

	typedef	int									distance_t;
	const distance_t							InfiniteDistance = -1;
	typedef std::unordered_map<node_index_t, 
							distance_t>			distance_dict_t;

}

#endif
#include "FurthestNode.h"
#include "defs.h"
#include "Graph.h"
#include "Node.h"

namespace dhtoolkit
{

	FurthestNode::FurthestNode(graph_ptr_t graph, node_ptr_t origin) : BFS(graph), _distance(0)
	{
		_nodes.push_back(origin);
		run(origin);
	}

	FurthestNode::~FurthestNode()
	{
		//empty on purpose
	}

	node_ptr_collection_t FurthestNode::getFurthestNodes() const
	{
		return _nodes;
	}

	distance_t FurthestNode::getFurthestDistance() const
	{
		return _distance;
	}

	bool FurthestNode::nodeTraversal(node_ptr_t curNode, node_ptr_t prevNode, distance_t distanceFromOrigin)
	{
		//if we've found a new furthest node (equally far or further)...
		if (_distance <= distanceFromOrigin)
		{
			//if it is further
			if (_distance < distanceFromOrigin)
			{
				//update distance and clear node collection
				_distance = distanceFromOrigin;
				_nodes.clear();
			}
			//add newly found node to collection
			_nodes.push_back(curNode);
		}

		return true;
	}

} // namespace dhtoolkit
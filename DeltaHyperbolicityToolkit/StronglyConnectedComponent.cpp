#include "StronglyConnectedComponent.h"
#include "defs.h"
#include "Node.h"

namespace dhtoolkit
{

	StronglyConnectedComponent::StronglyConnectedComponent(graph_ptr_t graph, node_ptr_t origin) : BFS(graph), _scc(new node_unordered_set_t())
	{
		run(origin);
	}

	StronglyConnectedComponent::~StronglyConnectedComponent()
	{
		//empty
	}

	const node_unordered_set_ptr_t StronglyConnectedComponent::getNodes() const
	{
		return _scc;
	}

	bool StronglyConnectedComponent::nodeTraversal(const node_ptr_t curNode, const node_ptr_t prevNode, distance_t distanceFromOrigin)
	{
		_scc->emplace(curNode);
		return true;
	}

} // namespace dhtoolkit
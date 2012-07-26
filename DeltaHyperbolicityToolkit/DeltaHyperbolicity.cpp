#include "DeltaHyperbolicity.h"
#include "defs.h"
#include "GraphAlgorithms.h"

namespace graphs
{
	DeltaHyperbolicity::DeltaHyperbolicity(delta_t delta) : _delta(delta), _nodesAvailable(false), _nodes() 
	{
		//empty
	}

	DeltaHyperbolicity::DeltaHyperbolicity(delta_t delta, node_quad_t nodes) : _delta(delta), _nodesAvailable(true)
	{
		_nodes = nodes;
	}

	delta_t DeltaHyperbolicity::getDelta() const
	{
		return _delta;
	}

	const node_quad_t& DeltaHyperbolicity::getNodes() const
	{
		return _nodes;
	}
} // namespace graphs
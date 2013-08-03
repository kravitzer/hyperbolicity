#include "DeltaHyperbolicity.h"
#include "defs.h"
#include "GraphAlgorithms.h"
#include <string>

using namespace std;

namespace dhtoolkit
{
	DeltaHyperbolicity::DeltaHyperbolicity() : _delta(0), _nodesAvailable(false), _nodes() 
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

	void DeltaHyperbolicity::setDelta(delta_t delta)
	{
		_delta = delta;
	}

	const node_quad_t& DeltaHyperbolicity::getState() const
	{
		return _nodes;
	}

	void DeltaHyperbolicity::setState(const node_quad_t& state)
	{
		_nodes = state;
	}

	string DeltaHyperbolicity::printNodes() const
	{
		return _nodes.printNodes();
	}
} // namespace dhtoolkit
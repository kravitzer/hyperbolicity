#include "DeltaHyperbolicity.h"
#include "defs.h"
#include "Graph\GraphAlgorithms.h"
#include <string>

using namespace std;

namespace hyperbolicity
{
	DeltaHyperbolicity::DeltaHyperbolicity() : _delta(0), _nodesAvailable(false), _nodes() 
	{
		//empty
	}

	DeltaHyperbolicity::DeltaHyperbolicity(delta_t delta, const node_combination_t& nodes) : _delta(delta), _nodesAvailable(true)
	{
		_nodes = nodes;
	}

	DeltaHyperbolicity::DeltaHyperbolicity(DeltaHyperbolicity&& other)
	{
		swap(*this, other);
	}

	DeltaHyperbolicity& DeltaHyperbolicity::operator=(DeltaHyperbolicity other)
	{
		swap(*this, other);

		return *this;
	}

	delta_t DeltaHyperbolicity::getDelta() const
	{
		return _delta;
	}

	const node_combination_t& DeltaHyperbolicity::getState() const
	{
		return _nodes;
	}

	void DeltaHyperbolicity::set(delta_t delta, const node_combination_t& state)
	{
		_delta = delta;
		_nodes = state;
	}

	string DeltaHyperbolicity::printNodes() const
	{
		return _nodes.printNodes();
	}

	void swap(DeltaHyperbolicity& first, DeltaHyperbolicity& second)
	{
		//enable ADT
		using std::swap;

		swap(first._delta, second._delta);
		swap(first._nodesAvailable, second._nodesAvailable);
		swap(first._nodes, second._nodes);
	}
} // namespace hyperbolicity
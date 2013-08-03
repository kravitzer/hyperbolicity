#include "IDSweepMinExt.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include "DeltaHyperbolicityToolkit\NodeDistances.h"
#include "DeltaHyperbolicityToolkit\State.h"
#include <time.h>
#include <string>
#include <unordered_map>

using namespace std;

namespace dhtoolkit
{
    const distance_t MaxDistance = INT_MAX;
	const unsigned int DoubleSweepCacheSize = 500;
	const float PercentageOfSweeps = 0.25;

	IDSweepMinExt::IDSweepMinExt(const string& outputDir) : IGraphAlg(outputDir), _doubleSweeps(DoubleSweepCacheSize)
	{
		//empty
	}

	IDSweepMinExt::~IDSweepMinExt()
	{
		//empty
	}

	void IDSweepMinExt::initImpl(const node_quad_t&)
	{
		_isComplete = false;
		_doubleSweeps.clear();
		_irremovableNodes.clear();
		_nodeCountInSweeps.clear();
	}

	DeltaHyperbolicity IDSweepMinExt::stepImpl()
	{
		//first perform a double sweep
		GraphAlgorithms::DoubleSweepResult ds = GraphAlgorithms::DoubleSweep(_graph);
		if (_doubleSweeps.size() + 1 > DoubleSweepCacheSize)
		{
			pair<node_index_t, node_index_t> earliestDs = _doubleSweeps.front();
			--_nodeCountInSweeps[earliestDs.first];
			--_nodeCountInSweeps[earliestDs.second];
		}
		auto firstIt = _nodeCountInSweeps.find(ds.u->getIndex());
		auto secondIt = _nodeCountInSweeps.find(ds.v->getIndex());
		_nodeCountInSweeps[ds.u->getIndex()] = (firstIt == _nodeCountInSweeps.cend() ? 1 : firstIt->second + 1);
		_nodeCountInSweeps[ds.v->getIndex()] = (secondIt == _nodeCountInSweeps.cend() ? 1 : secondIt->second + 1);
		
		_doubleSweeps.push_back(pair<node_index_t, node_index_t>(ds.u->getIndex(), ds.v->getIndex()));

		node_ptr_t& v1 = ds.u;
		node_ptr_t& v2 = ds.v;
		distance_dict_t& v1Dists = ds.uDistances;
		distance_dict_t v2Dists = NodeDistances(_graph, v2).getDistances();

		distance_t distV1V2 = v1Dists[v2->getIndex()];
		distance_t distV1V3 = MaxDistance;
		distance_t distV2V3 = MaxDistance;
		
		initStep();

		for (unsigned int i = 0; i < _graph->size(); ++i)
		{
			node_ptr_t curNode = _graph->getNode(i);
			if ( (curNode == v1) || (curNode == v2) ) continue;

			distance_t distFromV1 = v1Dists[curNode->getIndex()];
			distance_t distFromV2 = v2Dists[curNode->getIndex()];

			processV3Candidate(curNode, distFromV1, distFromV2);
		}

		//select one node from the v3 candidates
		unsigned int index = rand() % _v3Candidates.size();
		node_ptr_t v3 = _v3Candidates[index];
		distV1V3 = v1Dists[v3->getIndex()];
		distV2V3 = v2Dists[v3->getIndex()];

		//calculate distances for the newly selected v3
		distance_dict_t v3Dists = NodeDistances(_graph, v3).getDistances();

		node_ptr_t v4;
		delta_t maxDelta = -1;
		for (unsigned int i = 0; i < _graph->size(); ++i)
		{
			node_ptr_t curNode = _graph->getNode(i);
			if ( (curNode == v1) || (curNode == v2) || (curNode == v3) ) continue;

			distance_t distFromV1 = v1Dists[curNode->getIndex()];
			distance_t distFromV2 = v2Dists[curNode->getIndex()];
			distance_t distFromV3 = v3Dists[curNode->getIndex()];

            if ( (InfiniteDistance == distFromV1) || (InfiniteDistance == distFromV2) || (InfiniteDistance == distFromV3) ) continue;

			distance_t d1 = distV1V2 + distFromV3;
			distance_t d2 = distV1V3 + distFromV2;
			distance_t d3 = distV2V3 + distFromV1;

			delta_t curDelta = GraphAlgorithms::CalculateDeltaFromDistances(d1, d2, d3);
			if (curDelta > maxDelta)
			{
				maxDelta = curDelta;
				v4 = curNode;
			}
		}

		//just before returning the results, see if we can remove cycles
		delta_t deltaFromCycle = runOnSweepCollection();
		//make sure that after (potentially) removing nodes, the graph still has enough nodes to run on (otherwise we're done)
		_isComplete = (_graph->size() < State::size());
		if (deltaFromCycle > maxDelta) return DeltaHyperbolicity(deltaFromCycle, node_quad_t(_graph->getNode(0), _graph->getNode(0), _graph->getNode(0), _graph->getNode(0)));

		//return current step's result
		node_quad_t state(v1, v2, v3, v4);
		return DeltaHyperbolicity(maxDelta, state);
	}

	bool IDSweepMinExt::isComplete() const
	{
		//we can always run one more...
		return _isComplete;
	}

	delta_t IDSweepMinExt::runOnSweepCollection()
	{
		//make sure there's enough "history" to even have a node passing the threshold
		if (_doubleSweeps.size() < DoubleSweepCacheSize * PercentageOfSweeps) return 0;

		//iterate over node counts and look for one that reached the threshold
		delta_t bestDelta = 0;
		for (auto it = _nodeCountInSweeps.cbegin(); it != _nodeCountInSweeps.cend(); ++it)
		{
			//if current node is not marked as irremovable, and reached the threshold of iterations, remove cycles for that node
			if ( (_irremovableNodes.find(_graph->getNode(it->first)->getLabel()) == _irremovableNodes.cend()) && 
				(it->second >= DoubleSweepCacheSize * PercentageOfSweeps) )
			{
				delta_t delta;
				if (GraphAlgorithms::removeCycle(_graph, _graph->getNode(it->first), delta, _irremovableNodes))
				{
					_doubleSweeps.clear();
					_nodeCountInSweeps.clear();
					return delta;
				}
				else if (bestDelta < delta)
				{
					bestDelta = delta;
				}
			}
		}

		return 0;
	}
} // namespace dhtoolkit
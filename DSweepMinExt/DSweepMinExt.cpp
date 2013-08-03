#include "DSweepMinExt.h"
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

	DSweepMinExt::DSweepMinExt(const string& outputDir) : IGraphAlg(outputDir), _doubleSweeps(DoubleSweepCacheSize)
	{
		//empty
	}

	DSweepMinExt::~DSweepMinExt()
	{
		//empty
	}

	void DSweepMinExt::initImpl(const node_quad_t&)
	{
		_isComplete = false;
		_doubleSweeps.clear();
		_irremovableNodes.clear();
		_nodeCountInSweeps.clear();
	}

	DeltaHyperbolicity DSweepMinExt::stepImpl()
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
		node_ptr_collection_t v3Candidates;

		for (unsigned int i = 0; i < _graph->size(); ++i)
		{
			node_ptr_t curNode = _graph->getNode(i);
			if ( (curNode == v1) || (curNode == v2) ) continue;

			distance_t distFromV1 = v1Dists[curNode->getIndex()];
			distance_t distFromV2 = v2Dists[curNode->getIndex()];
            if ( (InfiniteDistance == distFromV1) || (InfiniteDistance == distFromV2) ) continue;

			if (distV1V3 == MaxDistance && distV2V3 == MaxDistance)
			{
				distV1V3 = distFromV1;
				distV2V3 = distFromV2;
				v3Candidates.push_back(curNode);
				continue;
			}

			//if ( (distFromV1 <= distFromV2 && distFromV2 <= distFromV1 + 1) || (distFromV2 <= distFromV1 && distFromV1 <= distFromV2 + 1) )
			if (distFromV1 + distFromV2 <= distV2V3 + distV1V3)
			{
				//if ( (distFromV2 <= distV2V3) && (distFromV1 <= distV1V3) )
				{
					//if ( (distFromV2 < distV2V3) || (distFromV1 < distV1V3) )
					if (distFromV1 + distFromV2 < distV2V3 + distV1V3)
					{
						distV2V3 = distFromV2;
						distV1V3 = distFromV1;

						v3Candidates.clear();						
					}
					v3Candidates.push_back(curNode);
				}
			}
		}

		//select one node from the v3 candidates
		unsigned int index = rand() % v3Candidates.size();
		node_ptr_t v3 = v3Candidates[index];
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

	bool DSweepMinExt::isComplete() const
	{
		//we can always run one more...
		return _isComplete;
	}

	delta_t DSweepMinExt::runOnSweepCollection()
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
				if (removeCycle(_graph->getNode(it->first), delta))
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

	bool DSweepMinExt::removeCycle(node_ptr_t origin, delta_t& delta)
	{
		//the chains of nodes from each side of origin to be deleted, if possible (excluding origin)
		node_ptr_collection_t chain1, chain2;

		node_weak_ptr_collection_t neighbors = origin->getEdges();
		if (neighbors.size() != 2) return 0;

		node_weak_ptr_t n1 = neighbors[0];
		node_weak_ptr_t n2 = neighbors[1];

		//the nodes that end the cycle
		node_ptr_t s, t;

		//start walking on chain1
		node_weak_ptr_t prev = origin;
		node_weak_ptr_t cur1 = neighbors[0];
		node_weak_ptr_collection_t curNeighbors = cur1.lock()->getEdges();
		for (; curNeighbors.size() == 2; curNeighbors = cur1.lock()->getEdges())
		{
			chain1.push_back(cur1.lock());

			node_weak_ptr_t tmp = cur1;
			cur1 = (curNeighbors[0].lock() == prev.lock() ? curNeighbors[1] : curNeighbors[0]);
			prev = tmp;

			//check extreme case: graph is a cycle?
			if (cur1.lock() == neighbors[1].lock())
			{
				//graph is a single cycle!
				delta_t curDelta = cycleDelta(_graph->size());
				_graph.reset(new Graph(_graph->getTitle()));
				delta = curDelta;
				return true;
			}
		}

		//start walking on chain2
		prev = origin;
		node_weak_ptr_t cur2 = neighbors[1];
		curNeighbors = cur2.lock()->getEdges();
		for (; curNeighbors.size() == 2; curNeighbors = cur2.lock()->getEdges())
		{
			chain2.push_back(cur2.lock());

			node_weak_ptr_t tmp = cur2;
			cur2 = (curNeighbors[0].lock() == prev.lock() ? curNeighbors[1] : curNeighbors[0]);
			prev = tmp;
		}

		NodeDistances distCalculator(_graph, cur1.lock());
		distance_t stDist = distCalculator.getDistance(cur2.lock());

		if (stDist == chain1.size() + chain2.size() + 2)
		{
			//we cannot remove the cycle as it may affect the value of delta

			//add this chain's nodes to the irremovable set
			_irremovableNodes.insert(origin->getLabel());
			for (auto it = chain1.cbegin(); it != chain1.cend(); ++it)
			{
				_irremovableNodes.insert((*it)->getLabel());
			}
			for (auto it = chain2.cbegin(); it != chain2.cend(); ++it)
			{
				_irremovableNodes.insert((*it)->getLabel());
			}

			//calculate maximal cycle size
			size_t cycleSize = (chain1.size() + chain2.size() + 2) * 2;
			delta = cycleDelta(cycleSize);
			return false;
		}
		else if (stDist < chain1.size() + chain2.size() + 2)
		{
			//calculate cycle size
			size_t cycleSize = chain1.size() + chain2.size() + 2 + stDist;

			//remove cycle!
			_graph->unmarkNodes();
			origin->mark();
			for (auto it = chain1.begin(); it != chain1.cend(); ++it) (*it)->mark();
			for (auto it = chain2.begin(); it != chain2.cend(); ++it) (*it)->mark();
			_graph->deleteMarkedNodes();

			delta = cycleDelta(cycleSize);
			return true;
		}
		else
		{
			//impossible - distance from s to t is at most the chains' length + 1 (origin)
			throw exception("Impossible distance received");
		}
	}

	delta_t DSweepMinExt::cycleDelta(size_t length) const
	{
		if (length % 4 == 1) return length/4.0 - 0.5;
		return length/4.0;
	}



	IGraphAlg* CreateAlgorithm(const string& outputDir)
	{
		//initialize random seed (necessary before calling DoubleSweep() ).
		srand(static_cast<unsigned int>(time(nullptr)));

		IGraphAlg* alg = new DSweepMinExt(outputDir);
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
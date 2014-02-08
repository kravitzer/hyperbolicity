#include "LargeDistances.h"
#include "Graph\defs.h"
#include "Graph\DeltaHyperbolicity.h"
#include "Graph\GraphAlgorithms.h"
#include "Graph\NodeDistances.h"
#include "Graph\FurthestNode.h"
#include <time.h>
#include <string>
#include <vector>
#include <math.h>

using namespace std;

const unsigned int MinDistanceDifferenceFromDistanceFound = 4;

namespace dhtoolkit
{
	LargeDistances::LargeDistances() : IGraphAlg(), _minDistance(0), _isComplete(false), _minNumberOfPairs(0)
	{
		
	}

	LargeDistances::~LargeDistances()
	{
		//empty
	}

	void LargeDistances::initImpl(const node_combination_t&)
	{
		node_index_t randNode = rand() % _graph->size();
		FurthestNode fn(_graph, _graph->getNode(randNode));
		node_ptr_collection_t furthestNodes = fn.getFurthestNodes();

		_nodeSet.clear();
		node_index_t originNode = furthestNodes[rand() % furthestNodes.size()]->getIndex();
		_nodeSet.insert(originNode);
		NodeDistances distanceCalc(_graph, _graph->getNode(originNode));
		_dists[originNode] = distanceCalc.getDistances();
		_minNumberOfPairs = static_cast<unsigned int>(_graph->size() * sqrt(_graph->size()));
		_pairsCollected = 0;
	}

	DeltaHyperbolicity LargeDistances::stepImpl()
	{	
		while ((_pairsCollected <= _minNumberOfPairs) && (_nodeSet.size() > 0))
		{
			auto nodeIt = _nodeSet.cbegin();
			node_index_t nextNodeIndex = *nodeIt;
			_nodeSet.erase(nodeIt);
			_processedNodeSet.insert(nextNodeIndex);

			if (_dists.cend() == _dists.find(nextNodeIndex)) throw exception("Distance vector doesn't exist for node");
			auto& distances = _dists[nextNodeIndex];

			for (distance_dict_t::const_iterator it = _dists[nextNodeIndex].cbegin(); it != _dists[nextNodeIndex].cend(); ++it)
			{
				node_index_t curIndex = it->first;

				if (static_cast<unsigned int>(it->second) > _minDistance + MinDistanceDifferenceFromDistanceFound)
				{
					//new maximal distance is found in graph! 
				
					//remove distances that are now outside the range
					for (distance_t curDistance = _minDistance; static_cast<unsigned int>(curDistance) < it->second - MinDistanceDifferenceFromDistanceFound; ++curDistance)
					{
						_pairsByDistance.erase(curDistance);
					}

					//store new minimal distance
					_minDistance = it->second - MinDistanceDifferenceFromDistanceFound;

					//add pair to pair map
					_pairsByDistance[it->second].push_back(std::pair<node_index_t, node_index_t>(nextNodeIndex, it->first));
					++_pairsCollected;
					//add new node to queue
					if (0 == _processedNodeSet.count(it->first) && _dists.cend() == _dists.find(it->first))
					{
						_nodeSet.insert(it->first);
						NodeDistances distanceCalc(_graph, _graph->getNode(it->first));
						_dists[it->first] = distanceCalc.getDistances();
					}
				}
				else if (static_cast<unsigned int>(it->second) >= static_cast<unsigned int>(_minDistance) && static_cast<unsigned int>(it->second) <= static_cast<unsigned int>(_minDistance )+ MinDistanceDifferenceFromDistanceFound)
				{
					//distance is in range, keep the pair
					_pairsByDistance[it->second].push_back(std::pair<node_index_t, node_index_t>(nextNodeIndex, it->first));
					++_pairsCollected;
					//add new node to queue
					if (0 == _processedNodeSet.count(it->first) && _dists.cend() == _dists.find(it->first))
					{
						_nodeSet.insert(it->first);
						NodeDistances distanceCalc(_graph, _graph->getNode(it->first));
						_dists[it->first] = distanceCalc.getDistances();
					}
				}
				//else do nothing, keep iteration
			}
		}


		delta_t bestDelta = 0;
		node_combination_t bestState;
		//iterate all pairs of pairs of nodes and calculate their delta
		for (auto it1 = _pairsByDistance.cbegin(); it1 != _pairsByDistance.cend(); ++it1)
		{
			for (auto it2 = it1; it2 != _pairsByDistance.cend(); ++it2)
			{
				for (unsigned int i1 = 0; i1 < it1->second.size(); ++i1)
				{
					unsigned int initialIndex = (it1 == it2 ? i1+1 : 0);
					for (unsigned int i2 = initialIndex; i2 < it2->second.size(); ++i2)
					{
						if (0 == _dists.count(it1->second[i1].first) ||
							0 == _dists.count(it1->second[i1].second) ||
							0 == _dists.count(it2->second[i2].first) ||
							0 == _dists.count(it2->second[i2].first)) throw exception("Distances do not exist for this node!");

						//calculate d1, d2, d3 and then delta
						unsigned int d1 = _dists[it1->second[i1].first][it1->second[i1].second] + _dists[it2->second[i2].first][it2->second[i2].second];
						unsigned int d2 = _dists[it1->second[i1].first][it2->second[i2].first] + _dists[it1->second[i1].second][it2->second[i2].second];
						unsigned int d3 = _dists[it1->second[i1].first][it2->second[i2].second] + _dists[it2->second[i2].first][it1->second[i1].second];
						delta_t curDelta = GraphAlgorithms::CalculateDeltaFromDistances(d1, d2, d3);

						if (curDelta > bestDelta)
						{
							bestDelta = curDelta;
							bestState.reset(_graph->getNode(it1->second[i1].first), _graph->getNode(it1->second[i1].second),
											_graph->getNode(it2->second[i2].first), _graph->getNode(it2->second[i2].second));
						}
					}
				}
			}
		}

		_isComplete = true;
		return DeltaHyperbolicity(bestDelta, bestState);
	}
	
	bool LargeDistances::isComplete() const
	{
		return _isComplete;
	}



	IGraphAlg* CreateAlgorithm()
	{
		//initialize random seed (necessary before calling DoubleSweep() )
		srand(static_cast<unsigned int>(time(nullptr)));

		IGraphAlg* alg = new LargeDistances();
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
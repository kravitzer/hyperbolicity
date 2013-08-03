#include "BruteForce.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include "DeltaHyperbolicityToolkit\State.h"
#include "DeltaHyperbolicityToolkit\Except.h"
#include "DeltaHyperbolicityToolkit\NodeDistances.h"
#include <time.h>
#include <string>

using namespace std;

namespace dhtoolkit
{
	BruteForce::BruteForce(const string& outputDir) : IGraphAlg(outputDir)
	{
		//empty
	}

	BruteForce::~BruteForce()
	{
		//empty
	}

	bool BruteForce::isComplete() const
	{
		return ( (_v4 == _graph->size() - node_quad_t::size() + 3) &&
				 (_v3 == _graph->size() - node_quad_t::size() + 2) &&
				 (_v2 == _graph->size() - node_quad_t::size() + 1) &&
				 (_v1 == _graph->size() - node_quad_t::size() + 0) );
	}

	void BruteForce::initImpl(const node_quad_t&)
	{
		_v1 = 0;
		_v2 = 1;
		_v3 = 2;
		_v4 = 2;	//each step advances the current state, so we start at state "-1" (i.e. one prior to the initial)

        _v1Distances = getDistancesForRemainingNodes(_v1);
        _v2Distances = getDistancesForRemainingNodes(_v2);
        _v3Distances = getDistancesForRemainingNodes(_v3);
	}

	DeltaHyperbolicity BruteForce::stepImpl()
	{
        node_index_t tmpv1 = _v1;
        node_index_t tmpv2 = _v2;
        node_index_t tmpv3 = _v3;

		node_quad_t curState = advanceState();
        if (tmpv3 != _v3) _v3Distances = getDistancesForRemainingNodes(_v3);
        if (tmpv1 != _v2) _v2Distances = getDistancesForRemainingNodes(_v2);
        if (tmpv1 != _v1) _v1Distances = getDistancesForRemainingNodes(_v1);

        distance_t d1 = _v1Distances[_v2] + _v3Distances[_v4];
        distance_t d2 = _v1Distances[_v3] + _v2Distances[_v4];
        distance_t d3 = _v1Distances[_v4] + _v2Distances[_v3];
		delta_t curDelta = GraphAlgorithms::CalculateDeltaFromDistances(d1, d2, d3);

		return DeltaHyperbolicity(curDelta, curState);
	}

	node_quad_t BruteForce::advanceState()
	{
		do
		{
			//increment v4
			_v4 = (_v4 + 1) % _graph->size();
			if (0 == _v4)
			{
				//v4 overlapped, increment v3
				_v3 = (_v3 + 1) % (_graph->size() - 1);
				if (0 == _v3)
				{
					//v3 overlapped, increment v3
					_v2 = (_v2 + 1) % (_graph->size() - 2);
					if (0 == _v2)
					{
						//v2 overlapped, increment v3
						_v1 = (_v1 + 1) % (_graph->size() - 3);
						//it's impossible for v1 to overlap as well - just make sure
						if (0 == _v1) throw AlgorithmCompletedException("Brute force algorithm completed, yet another step was required");
						_v2 = _v1 + 1;
					}
					_v3 = _v2 + 1;
				}
				_v4 = _v3 + 1;
			}
		} while (!node_quad_t::isStateValid( _graph->getNode(_v1), _graph->getNode(_v2), _graph->getNode(_v3), _graph->getNode(_v4) ) );

		return node_quad_t( _graph->getNode(_v1), _graph->getNode(_v2), _graph->getNode(_v3), _graph->getNode(_v4) );
	}

    distance_dict_t BruteForce::getDistancesForRemainingNodes(node_index_t curIndex)
    {
        node_ptr_collection_t nodesToSearch;
        for (node_index_t i = curIndex + 1; i < _graph->size(); ++i)
        {
            nodesToSearch.push_back(_graph->getNode(i));
        }

        NodeDistances nd(_graph, _graph->getNode(curIndex));
        return nd.getDistances(nodesToSearch);
    }



	IGraphAlg* CreateAlgorithm(const string& outputDir)
	{
		IGraphAlg* alg = new BruteForce(outputDir);
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
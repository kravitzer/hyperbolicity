#include "DSweep.h"
#include "Graph\defs.h"
#include "Graph\DeltaHyperbolicity.h"
#include "Graph\GraphAlgorithms.h"
#include "Graph\NodeDistances.h"
#include <time.h>
#include <string>

using namespace std;

namespace dhtoolkit
{
	DSweep::DSweep() : IGraphAlg() 
	{
		//empty
	}

	DSweep::~DSweep()
	{
		//empty
	}

	DeltaHyperbolicity DSweep::stepImpl() 
	{
		//first perform a double sweep
		GraphAlgorithms::DoubleSweepResult ds = GraphAlgorithms::DoubleSweep(_graph);

		node_ptr_t& v1 = ds.u;
		node_ptr_t& v2 = ds.v;
		distance_dict_t& v1Dists = ds.uDistances;
		distance_dict_t v2Dists = NodeDistances(_graph, v2).getDistances();;

		distance_t distV1V2 = v1Dists[v2->getIndex()];
		distance_t distV1V3 = 0;
		distance_t distV2V3 = 0;
		node_ptr_collection_t v3Candidates;

		for (unsigned int i = 0; i < _graph->size(); ++i)
		{
			node_ptr_t curNode = _graph->getNode(i);
			if ( (curNode == v1) || (curNode == v2) ) continue;

			distance_t distFromV1 = v1Dists[curNode->getIndex()];
			distance_t distFromV2 = v2Dists[curNode->getIndex()];

			if ( (distFromV1 <= distFromV2 && distFromV2 <= distFromV1 + 1) || (distFromV2 <= distFromV1 && distFromV1 <= distFromV2 + 1) )
			{
				if ( (distFromV2 >= distV2V3) && (distFromV1 >= distV1V3) )
				{
					if ( (distFromV2 > distV2V3) || (distFromV1 > distV1V3) )
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

		//calculate distances for the newly selected v3
		distance_dict_t v3Dists = NodeDistances(_graph, v3).getDistances();

		node_ptr_t v4;
		delta_t maxDelta = 0;
		for (unsigned int i = 0; i < _graph->size(); ++i)
		{
			node_ptr_t curNode = _graph->getNode(i);
			if ( (curNode == v1) || (curNode == v2) || (curNode == v3) ) continue;

			distance_t distFromV1 = v1Dists[curNode->getIndex()];
			distance_t distFromV2 = v2Dists[curNode->getIndex()];
			distance_t distFromV3 = v3Dists[curNode->getIndex()];

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

		node_combination_t state(v1, v2, v3, v4);
		return DeltaHyperbolicity(maxDelta, state);
	}

	void DSweep::initImpl(const node_combination_t&)
	{
		//empty
	}

	bool DSweep::isComplete() const
	{
		//we can always run one more...
		return false;
	}



	IGraphAlg* CreateAlgorithm()
	{
		//initialize random seed (necessary before calling DoubleSweep() ).
		srand(static_cast<unsigned int>(time(nullptr)));

		IGraphAlg* alg = new DSweep();
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
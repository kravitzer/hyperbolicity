#include "DSweep.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include <time.h>

namespace graphs
{
	DeltaHyperbolicity DSweep::run(const graph_ptr_t graph) 
	{
		//first perform a double sweep
		GraphAlgorithms::DoubleSweepResult ds = GraphAlgorithms::DoubleSweep(graph);

		node_ptr_t& v1 = ds.u;
		node_ptr_t& v2 = ds.v;
		distance_dict_t& v1Dists = ds.uDistances;
		distance_dict_t v2Dists = GraphAlgorithms::Dijkstra(graph, v2);

		distance_t distV1V2 = v1Dists[v2->getIndex()];
		distance_t distV1V3 = 0;
		distance_t distV2V3 = 0;
		node_collection_t v3Candidates;

		for (unsigned int i = 0; i < graph->size(); ++i)
		{
			node_ptr_t curNode = graph->getNode(i);
			if ( (curNode == ds.u) || (curNode == ds.v) ) continue;

			distance_t distFromU = v1Dists[curNode->getIndex()];
			distance_t distFromV = v2Dists[curNode->getIndex()];

			if ( (distFromU <= distFromV && distFromV <= distFromU + 1) || (distFromV <= distFromU && distFromU <= distFromV + 1) )
			{
				if ( (distFromU >= distV2V3) && (distFromV >= distV1V3) )
				{
					if ( (distFromU > distV2V3) || (distFromV > distV1V3) )
					{
						distV2V3 = distFromU;
						distV1V3 = distFromV;

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
		distance_dict_t v3Dists = GraphAlgorithms::Dijkstra(graph, v3);

		node_ptr_t v4;
		delta_t maxDelta = 0;
		for (unsigned int i = 0; i < graph->size(); ++i)
		{
			node_ptr_t curNode = graph->getNode(i);
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

		node_quad_t state;
		state[0] = v1;
		state[1] = v2;
		state[2] = v3;
		state[3] = v4;

		return DeltaHyperbolicity(maxDelta, state);
	}

	DeltaHyperbolicity DSweep::runWithInitialState(const graph_ptr_t graph, const node_quad_t&)
	{
		//initial state is ignored
		return run(graph);
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
} // namespace graphs
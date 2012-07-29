#include "BruteForce.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include "DeltaHyperbolicityToolkit\State.h"
#include <time.h>

namespace dhtoolkit
{
	DeltaHyperbolicity BruteForce::runImpl(const graph_ptr_t graph) 
	{
		delta_t maxDelta = 0;
		node_quad_t maxState;
		node_quad_t curState;

		for (unsigned int i = 0; i <= graph->size() - node_quad_t::size(); ++i)
		{
			curState[0] = graph->getNode(i);
			for (unsigned int j = i+1; j <= graph->size() - node_quad_t::size() + 1; ++j)
			{
				curState[1] = graph->getNode(j);
				for (unsigned int k = j+1; k <= graph->size() - node_quad_t::size() + 2; ++k)
				{
					curState[2] = graph->getNode(k);
					for (unsigned int l = k+1; l <= graph->size() - node_quad_t::size() + 3; ++l)
					{
						curState[3] = graph->getNode(l);

						delta_t curDelta = GraphAlgorithms::CalculateDelta(graph, curState);
						if (curDelta > maxDelta)
						{
							//keep new max-values
							maxDelta = curDelta;
							maxState = curState;
						}
					}
				}
			}
		}

		return DeltaHyperbolicity(maxDelta, maxState);
	}

	DeltaHyperbolicity BruteForce::runWithInitialStateImpl(const graph_ptr_t graph, const node_quad_t&)
	{
		//initial state is ignored, as this algorithm calculates all states anyways...
		return runImpl(graph);
	}



	IGraphAlg* CreateAlgorithm()
	{
		IGraphAlg* alg = new BruteForce();
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
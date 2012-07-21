#include "BruteForce.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include <time.h>

namespace graphs
{
	DeltaHyperbolicity BruteForce::run(const graph_ptr_t graph) 
	{
		delta_t maxDelta = 0;
		node_quad_t maxState;
		node_quad_t curState;

		for (unsigned int i = 0; i <= graph->size() - NodeQuadCount; ++i)
		{
			curState[0] = graph->getNode(i);
			for (unsigned int j = i+1; j <= graph->size() - NodeQuadCount + 1; ++j)
			{
				curState[1] = graph->getNode(j);
				for (unsigned int k = j+1; k <= graph->size() - NodeQuadCount + 2; ++k)
				{
					curState[2] = graph->getNode(k);
					for (unsigned int l = k+1; l <= graph->size() - NodeQuadCount + 3; ++l)
					{
						curState[3] = graph->getNode(l);

						delta_t curDelta = GraphAlgorithms::CalculateDelta(graph, curState);
						if (curDelta > maxDelta)
						{
							//keep new max-values
							maxDelta = curDelta;
							for (unsigned int m = 0; m < NodeQuadCount; ++m)
							{
								maxState[m] = curState[m];
							}
						}
					}
				}
			}
		}

		return DeltaHyperbolicity(maxDelta, maxState);
	}

	DeltaHyperbolicity BruteForce::runWithInitialState(const graph_ptr_t graph, const node_quad_t&)
	{
		//initial state is ignored
		return run(graph);
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
} // namespace graphs
#include "DDSweep.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include <time.h>

namespace graphs
{
	DeltaHyperbolicity DDSweep::run(const graph_ptr_t graph) 
	{
		GraphAlgorithms::DoubleSweepResult ds1 = GraphAlgorithms::DoubleSweep(graph);
		GraphAlgorithms::DoubleSweepResult ds2 = GraphAlgorithms::DoubleSweep(graph);
		
		unsigned int numOfTrials = 1;

		while ( (ds1.u == ds2.u) || (ds1.v == ds2.u) || (ds1.u == ds2.v) || (ds1.v == ds2.v) )
		{
			if (numOfTrials >= MaxNumOfTrials) throw std::exception("Exceeded the maximal number of trials allowed!");
			ds2 = GraphAlgorithms::DoubleSweep(graph);
			++numOfTrials;
		}

		node_quad_t state = {0};
		state[0] = ds1.u;
		state[1] = ds1.v;
		state[2] = ds2.u;
		state[3] = ds2.v;

		delta_t delta = GraphAlgorithms::CalculateDelta(graph, state);
		return DeltaHyperbolicity(delta, state);
	}

	DeltaHyperbolicity DDSweep::runWithInitialState(const graph_ptr_t graph, const node_quad_t&)
	{
		//initial state is ignored
		return run(graph);
	}



	IGraphAlg* CreateAlgorithm()
	{
		//initialize random seed (necessary before calling DoubleSweep() )
		srand(static_cast<unsigned int>(time(nullptr)));

		IGraphAlg* alg = new DDSweep();
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace graphs
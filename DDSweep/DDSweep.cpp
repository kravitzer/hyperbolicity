#include "DDSweep.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include <time.h>
#include <string>

using namespace std;

namespace dhtoolkit
{
	DDSweep::DDSweep(const string& outputDir) : IGraphAlg(outputDir) 
	{
		//empty
	}

	DeltaHyperbolicity DDSweep::runImpl(const graph_ptr_t graph) 
	{
		GraphAlgorithms::DoubleSweepResult ds1 = GraphAlgorithms::DoubleSweep(graph);
		GraphAlgorithms::DoubleSweepResult ds2 = GraphAlgorithms::DoubleSweep(graph);
		
		unsigned int numOfTrials = 1;

		while ( (ds1.u == ds2.u) || (ds1.v == ds2.u) || (ds1.u == ds2.v) || (ds1.v == ds2.v) || (ds1.u == ds1.v) || (ds2.u == ds2.v) )
		{
			if (numOfTrials >= MaxNumOfTrials) throw std::exception("Exceeded the maximal number of trials allowed!");
			ds2 = GraphAlgorithms::DoubleSweep(graph);
			++numOfTrials;
		}

		node_quad_t state(ds1.u, ds1.v, ds2.u, ds2.v);
		delta_t delta = GraphAlgorithms::CalculateDelta(graph, state);
		return DeltaHyperbolicity(delta, state);
	}

	DeltaHyperbolicity DDSweep::runWithInitialStateImpl(const graph_ptr_t graph, const node_quad_t& initialState)
	{
		return runAndReturnBetter(graph, initialState);
	}

	void DDSweep::reset()
	{
		//empty
	}



	IGraphAlg* CreateAlgorithm(const string& outputDir)
	{
		//initialize random seed (necessary before calling DoubleSweep() )
		srand(static_cast<unsigned int>(time(nullptr)));

		IGraphAlg* alg = new DDSweep(outputDir);
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
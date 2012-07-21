#include "DDSweep.h"
#include "DeltaHyperbolicity.h"

namespace graphs
{
	DeltaHyperbolicity DDSweep::run(const graph_ptr_t graph) 
	{
		return DeltaHyperbolicity(1.5);
	}

	DeltaHyperbolicity DDSweep::runWithInitialState(const graph_ptr_t graph, const node_quad_t& initialState)
	{ 
		return DeltaHyperbolicity(3);
	}



	IGraphAlg* CreateAlgorithm()
	{
		IGraphAlg* alg = new DDSweep();
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace graphs
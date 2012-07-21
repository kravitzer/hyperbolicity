#ifndef DELTAHYPER_DSWEEP_H
#define DELTAHYPER_DSWEEP_H

#include "DeltaHyperbolicityToolkit\IGraphAlg.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"

namespace graphs
{
	class DSweep : public IGraphAlg
	{
	public:
		DSweep() : IGraphAlg() {}
		virtual DeltaHyperbolicity run(const graph_ptr_t graph);
		virtual DeltaHyperbolicity runWithInitialState(const graph_ptr_t graph, const node_quad_t& initialState);

	private:
		//The maximal number of trials to get 4 different nodes out of separate double-sweeps
		static const unsigned int MaxNumOfTrials = 50;
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm();
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace graphs

#endif //DELTAHYPER_DSWEEP_H
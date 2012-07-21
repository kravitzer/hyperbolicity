#ifndef DELTAHYPER_DDSWEEP_H
#define DELTAHYPER_DDSWEEP_H

#include "IGraphAlg.h"
#include "DeltaHyperbolicity.h"

namespace graphs
{
	class DDSweep : public IGraphAlg
	{
	public:
		DDSweep() : IGraphAlg() {}
		virtual DeltaHyperbolicity run(const graph_ptr_t graph);
		virtual DeltaHyperbolicity runWithInitialState(const graph_ptr_t graph, const node_quad_t& initialState);
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm();
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace graphs

#endif //DELTAHYPER_DDSWEEP_H
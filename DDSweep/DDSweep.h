/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_DDSWEEP_H
#define DELTAHYPER_DDSWEEP_H

#include "DeltaHyperbolicityToolkit\IGraphAlg.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"

namespace graphs
{
	/*
	 * @brief	Implementation for the double-double-sweep algorithm. This algorithm runs a double-sweep on the graph,
	 *			and then repeatedly runs another double-sweep until receiving 4 total different nodes. It then uses these
	 *			4 nodes to calculate the delta hyperbolicity, and returns the result.
	 */
	class DDSweep : public IGraphAlg
	{
	public:
		DDSweep() : IGraphAlg() {}

		/*
		 * @brief	See documentation in IGraphAlg and in the class description above.
		 */
		virtual DeltaHyperbolicity run(const graph_ptr_t graph);
		virtual DeltaHyperbolicity runWithInitialState(const graph_ptr_t graph, const node_quad_t& initialState);

	private:
		//The maximal number of trials to get 4 different nodes out of separate double-sweeps
		static const unsigned int MaxNumOfTrials = 50;
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm();
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace graphs

#endif //DELTAHYPER_DDSWEEP_H
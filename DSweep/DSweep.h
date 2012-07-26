/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_DSWEEP_H
#define DELTAHYPER_DSWEEP_H

#include "DeltaHyperbolicityToolkit\IGraphAlg.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"

namespace graphs
{
	/*
	 * @brief	Implementation for the single-double-sweep algorithm. It performs a single double sweep to obtain the first 2 nodes. Then
	 *			it runs on the entire graph, looking for a third node that is as far as possible from both nodes yet the distances from the
	 *			two nodes differ by at most 1 (e.g. distance from v1 is 5, from v2 is 6...). If there are many "furthest balanced" nodes,
	 *			it chooses one randomly. Then, it simply goes over all nodes one more time, selecting the node that produces the maximal
	 *			delta hyperbolicity value (at this point we already have all the distances from v1, v2, v3).
	 */
	class DSweep : public IGraphAlg
	{
	public:
		DSweep() : IGraphAlg() {}

		/*
		 * @brief	See documentation in IGraphAlg and in the class description above.
		 */
		virtual DeltaHyperbolicity runImpl(const graph_ptr_t graph);
		virtual DeltaHyperbolicity runWithInitialStateImpl(const graph_ptr_t graph, const node_quad_t& initialState);

	private:
		//The maximal number of trials to get 4 different nodes out of separate double-sweeps
		static const unsigned int MaxNumOfTrials = 50;
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm();
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace graphs

#endif //DELTAHYPER_DSWEEP_H
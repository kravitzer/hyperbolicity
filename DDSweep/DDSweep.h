/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_DDSWEEP_H
#define DELTAHYPER_DDSWEEP_H

#include "DeltaHyperbolicityToolkit\IGraphAlg.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include <string>

namespace dhtoolkit
{
	/*
	 * @brief	Implementation for the double-double-sweep algorithm. This algorithm runs a double-sweep on the graph,
	 *			and then repeatedly runs another double-sweep until receiving 4 total different nodes. It then uses these
	 *			4 nodes to calculate the delta hyperbolicity, and returns the result.
	 */
	class DDSweep : public IGraphAlg
	{
	public:
		DDSweep(const std::string& outputDir);

	private:
		/*
		 * @brief	See documentation in IGraphAlg and in the class description above.
		 */
		virtual DeltaHyperbolicity runImpl(const graph_ptr_t graph);
		virtual DeltaHyperbolicity runWithInitialStateImpl(const graph_ptr_t graph, const node_quad_t& initialState);
		virtual void reset();

		//The maximal number of trials to get 4 different nodes out of separate double-sweeps
		static const unsigned int MaxNumOfTrials = 50;
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm(const std::string& outputDir);
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace dhtoolkit

#endif //DELTAHYPER_DDSWEEP_H
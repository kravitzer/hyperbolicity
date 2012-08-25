/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_BRUTEFORCE_H
#define DELTAHYPER_BRUTEFORCE_H

#include "DeltaHyperbolicityToolkit\IGraphAlg.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include <string>

namespace dhtoolkit
{
	/*
	 * @brief	Implementation for the brute-force solution for finding delta hyperbolicity. Obviously takes a significant amount of time
	 *			even on small graphs, so it should only be run on VERY small graphs. Simply goes over all 4-node combinations,
	 *			looking for the one that produces that maximal delta hyperbolicity.
	 */
	class BruteForce : public IGraphAlg
	{
	public:
		BruteForce(const std::string& outputDir);

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

#endif //DELTAHYPER_BRUTEFORCE_H
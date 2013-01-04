/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_MDSWEEP_H
#define DELTAHYPER_MDSWEEP_H

#include "DeltaHyperbolicityToolkit\IGraphAlg.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include <string>

namespace dhtoolkit
{
	/*
	 * @brief	Implementation for the multi-double-sweep algorithm. This algorithm runs multiple double-sweeps on the
	 *			graph, and then calculates the delta hyperbolicity resulting from each pair of sweep results. It takes
	 *			the best pair as the result.
	 */
	class MDSweep : public IGraphAlg
	{
	public:
		MDSweep(const std::string& outputDir);

	private:
		/*
		 * @brief	See documentation in IGraphAlg and in the class description above.
		 */
		virtual DeltaHyperbolicity runImpl(const graph_ptr_t graph);
		virtual DeltaHyperbolicity runWithInitialStateImpl(const graph_ptr_t graph, const node_quad_t& initialState);
		virtual void reset();

		/*
		 * @param	numOfNodes	The number of nodes in the graph.
		 * @returns	The number of double sweeps to perform.
		 */
		unsigned int numOfDoubleSweeps(unsigned int numOfNodes) const;

		/*
		 * @returns true iff the two double sweep results have no nodes in common.
		 */
		bool areSweepsUnique(GraphAlgorithms::DoubleSweepResult& res1, GraphAlgorithms::DoubleSweepResult& res2);

		//The maximal number of trials to get 4 different nodes out of separate double-sweeps
		static const unsigned int MaxNumOfTrials = 50;
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm(const std::string& outputDir);
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace dhtoolkit

#endif //DELTAHYPER_DDSWEEP_H
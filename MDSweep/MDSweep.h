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
		virtual ~MDSweep();

	private:
		/*
		 * @brief	See documentation in IGraphAlg and in the class description above.
		 */
		virtual DeltaHyperbolicity stepImpl();
		virtual void initImpl(const node_quad_t& initialState);
		virtual bool isComplete() const;

		/*
		 * @returns true iff the two double sweep results have no nodes in common.
		 */
		bool areSweepsUnique(GraphAlgorithms::DoubleSweepResult& res1, GraphAlgorithms::DoubleSweepResult& res2);

		/*
		 * @returns	True if the given double-sweep is not already present in the _sweeps collection, false otherwise.
		 */
		bool isNewSweep(const GraphAlgorithms::DoubleSweepResult& ds) const;

		//The maximal number of trials to get 4 different nodes out of separate double-sweeps
		static const unsigned int MaxNumOfTrials = 50;

		//sweeps collected so far
		std::vector<GraphAlgorithms::DoubleSweepResult> _sweeps;
		//the distances from the "other" vertex of the double sweep to the rest of the nodes
		//i.e. _vDists[i] has the distances from _sweeps[i]'s second vertex to the rest of the graph
		std::vector<distance_dict_t> _vDists;
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm(const std::string& outputDir);
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace dhtoolkit

#endif //DELTAHYPER_DDSWEEP_H
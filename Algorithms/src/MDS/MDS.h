/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_MDSWEEP_H
#define DELTAHYPER_MDSWEEP_H

#include "Algorithm\IGraphAlg.h"
#include "Algorithm\DeltaHyperbolicity.h"
#include "Graph\GraphAlgorithms.h"
#include "Algorithm\HyperbolicityAlgorithms.h"
#include <string>

namespace dhtoolkit
{
	/*
	 * @brief	Implementation for the multi-double-sweep algorithm. This algorithm runs multiple double-sweeps on the
	 *			graph, and then calculates the delta hyperbolicity resulting from each pair of sweep results. It takes
	 *			the best pair as the result.
	 */
	class MDS : public IGraphAlg
	{
	public:
		MDS();
		virtual ~MDS();

	private:
		/*
		 * @brief	See documentation in IGraphAlg and in the class description above.
		 */
		virtual DeltaHyperbolicity stepImpl();
		virtual void initImpl(const node_combination_t& initialState);
		virtual bool isComplete() const;

		/*
		 * @brief	Finds a new, unique double-sweep result and adds it to the collection of results.
		 *			Also calculates the distances from the previous double sweep's v node to the rest of the graph.
		 */
		void prepareNextStep();

		/*
		 * @returns true iff the two double sweep results have no nodes in common.
		 */
		bool areSweepsUnique(HyperbolicityAlgorithms::DoubleSweepResult& res1, HyperbolicityAlgorithms::DoubleSweepResult& res2);

		/*
		 * @returns	True if the given double-sweep is not already present in the _sweeps collection, false otherwise.
		 */
		bool isNewSweep(const HyperbolicityAlgorithms::DoubleSweepResult& ds) const;

		//The maximal number of trials to get 4 different nodes out of separate double-sweeps
		static const unsigned int MaxNumOfTrials = 500;

		//sweeps collected so far
		std::vector<HyperbolicityAlgorithms::DoubleSweepResult> _sweeps;
		//the distances from the "other" vertex of the double sweep to the rest of the nodes
		//i.e. _vDists[i] has the distances from _sweeps[i]'s second vertex to the rest of the graph
		std::vector<distance_dict_t> _vDists;
		//we only decide that we're done if the maximal allowed number of trials to achieve a new unique double-sweep has been exceeded
		bool _isCompleted;
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm();
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace dhtoolkit

#endif //DELTAHYPER_DDSWEEP_H
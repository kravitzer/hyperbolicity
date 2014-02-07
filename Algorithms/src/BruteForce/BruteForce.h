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
		BruteForce();
		virtual ~BruteForce();
		bool isComplete() const;

	private:
		/*
		 * @brief	See documentation in IGraphAlg and in the class description above.
		 */
		virtual void initImpl(const node_combination_t&);
		virtual DeltaHyperbolicity stepImpl();

		/*
		 * @brief	Advances the current state to the next.
		 */
		node_combination_t advanceState();

        /*
         * @returns The distances from the node whose index is given, to every node with an index higher than its own.
         */
        distance_dict_t getDistancesForRemainingNodes(node_index_t curIndex);

		//The maximal number of trials to get 4 different nodes out of separate double-sweeps
		static const unsigned int MaxNumOfTrials = 50;

		//the indecis of the current nodes, iterated in order
		node_index_t _v1, _v2, _v3, _v4;
        distance_dict_t _v1Distances, _v2Distances, _v3Distances;
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm(const std::string& outputDir);
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace dhtoolkit

#endif //DELTAHYPER_BRUTEFORCE_H
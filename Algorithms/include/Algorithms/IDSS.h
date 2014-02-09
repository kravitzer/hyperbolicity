/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_IDSWEEPMINEXT_H
#define DELTAHYPER_IDSWEEPMINEXT_H

#include "Algorithm\IGraphAlg.h"
#include "Algorithm\DeltaHyperbolicity.h"
#include <string>
#include <boost\circular_buffer.hpp>
#include <unordered_map>
#include <unordered_set>

namespace dhtoolkit
{
	/*
	 * @brief	Abstract implementation for a single-double-sweep with minimum distance: It performs a single double sweep
	 *			to obtain the first 2 nodes. Then it runs on the entire graph, looking for a third node according to the 
	 *			candidates selected by the derived class' implementation (based on its distances from v1 and v2). After
	 *			choosing one of the candidates randomly, it simply goes over all nodes one more time, selecting the node
	 *			that produces the maximal delta hyperbolicity value (at this point we already have all the distances from v1, v2, v3).
	 *
	 *			This implementation also remembers the last p Double-Sweeps (p is a parameter) and at each step (after the p'th step):
	 *			For each node that appeard in at least q (also a parameter) of the Double-Sweeps, try to see if it is a part of a cycle and
	 *			remove it if possible.
	 */
	class IDSweepMinExt : public IGraphAlg
	{
	public:
		IDSweepMinExt();
		virtual ~IDSweepMinExt();

	protected:
		/*
		 * @brief	This method is run at the beginning of each step, to allow derived algorithm to perform step initialization.
		 */
		virtual void initStep() = 0;

		/*
		 * @brief	Either skips or adds the current v3 candidate to the collection of candidates. If it is better than all previously
		 *			existing candidates, the collection is cleared before the current candidate is added.
		 * @param	v3Candidate	The node considered to be added as a v3 candidate.
		 * @param	distFromV1	The distance of the current candidate from v1.
		 * @param	distFromV2	The distance of the current candidate from v2.
		 */
		virtual void processV3Candidate(node_ptr_t v3Candidate, distance_t distFromV1, distance_t distFromV2) = 0;

		//this collection is to be used by the derived class to hold the up-to-date v3 candidates for each step of the algorithm.
		node_ptr_collection_t _v3Candidates;

	private:
		/*
		 * @brief	See documentation in IGraphAlg and in the class description above.
		 */
		virtual DeltaHyperbolicity stepImpl();
		virtual void initImpl(const node_combination_t& initialState);
		virtual bool isComplete() const;

		/*
		 * @brief	Runs on the double-sweep collection and run the cycle removal algorithm if necessary
		 * @param	node	A node that's part of the largest cycle found (which thus yielded the largest delta).
		 * @returns	The delta of the largest cycle found when scanning for nodes to be removed (or 0 if none found).
		 */
		delta_t runOnSweepCollection(node_ptr_t& node);

		//collection of the last X double sweep results (X is a constant set in the cpp file)
		boost::circular_buffer<std::pair<node_index_t, node_index_t>> _doubleSweeps;
		//maps the node index to the number of times it appeared in the last double sweeps
		std::unordered_map<node_index_t, size_t> _nodeCountInSweeps;
		//are we done? (only in the case where we removed nodes such that the remaining graph has too few nodes)
		bool _isComplete;
		//this set holds the nodes that were tested and found irremovable (so there's no point in testing them on every step)
		//we save the label instead of the index because after we remove nodes, indices may change
		std::unordered_set<std::string> _irremovableNodes;
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm();
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace dhtoolkit

#endif //DELTAHYPER_IDSWEEPMINEXT_H
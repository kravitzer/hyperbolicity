/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_DSWEEPMINEXT_H
#define DELTAHYPER_DSWEEPMINEXT_H

#include "DeltaHyperbolicityToolkit\IGraphAlg.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include <string>
#include <boost\circular_buffer.hpp>
#include <unordered_map>
#include <unordered_set>

namespace dhtoolkit
{
	/*
	 * @brief	Implementation for the extended minimal double-sweep algorithm. Acts the same as the DSweepMin algorithm,
	 *			except that it remember the last p Double-Sweeps (p is a parameter) and at each step (after the p'th step):
	 *			for each node that appeard in at least p/2 of the DoubleSweeps, try to see if it is a part of a cycle and
	 *			remove it if possible.
	 *
	 *			The cycle removal algorithm is as follows:
	 *				Starting at node v, for as long as degree(v) == 2, go to the neighbor that you did not come from (on first step choose arbitrarily).
	 *				Of course, if we've somehow reached the first node - the graph is a simple cycle and we're done...
	 *				Do the same when choosing the other node on the first step.
	 *				We now reached two vertices, s & t, whose degree > 2. Check d(s, t).
	 *				If d(s, t) < number of nodes iterated (excluding s & t) then the nodes iterated (excluding s & t) may be removed!
	 *				Before removal, we calculate the delta value produced from the cycle (truncate(length / 4) if length % 4 != 1, 
	 *				or truncate(length / 4) - 0.5 otherwise). We consider this delta as well when returning the best delta found.
	 */
	class DSweepMinExt : public IGraphAlg
	{
	public:
		DSweepMinExt(const std::string& outputDir);
		virtual ~DSweepMinExt();

	private:
		/*
		 * @brief	See documentation in IGraphAlg and in the class description above.
		 */
		virtual DeltaHyperbolicity stepImpl();
		virtual void initImpl(const node_quad_t& initialState);
		virtual bool isComplete() const;

		/*
		 * @brief	Runs on the double-sweep collection and run the cycle removal algorithm if necessary
		 * @returns	The delta found from the cycle removed, or 0 if no cycle is removed.
		 */
		delta_t runOnSweepCollection();

		/*
		 * @brief	Removes the cycle that origin is a part of it, if really is a part of a cycle, and if cycle may be removed without
		 *			affecting the hyperbolicity. See class documentation for more details.
		 * @param	delta	Will be set to the delta of the cycle found (or 0 if not found).
		 * @returns	Whether a cycle was removed or not.
		 */
		bool removeCycle(node_ptr_t origin, delta_t& delta);

		/*
		 * @returns	The delta value of a cycle-graph whose length is given.
		 */
		delta_t cycleDelta(size_t length) const;

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

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm(const std::string& outputDir);
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace dhtoolkit

#endif //DELTAHYPER_DSWEEPMINEXT_H
/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_DSWEEPMINBALANCED_H
#define DELTAHYPER_DSWEEPMINBALANCED_H

#include "DeltaHyperbolicityToolkit\IGraphAlg.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include <string>
#include <boost\circular_buffer.hpp>
#include <unordered_map>
#include <unordered_set>

namespace dhtoolkit
{
	/*
	 * @brief	Same as DSweepMinExt except distances from v3 to v1 and to v2 must be balanced (i.e. abs(d(v1, v3)-d(v2, v3)) <= 1).
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

#endif //DELTAHYPER_DSWEEPMINBALANCED_H
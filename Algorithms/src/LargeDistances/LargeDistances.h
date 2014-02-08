/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_LARGEDISTANCES_H
#define DELTAHYPER_LARGEDISTANCES_H

#include "Graph\IGraphAlg.h"
#include "Graph\DeltaHyperbolicity.h"
#include "Graph\GraphAlgorithms.h"
#include <string>
#include <unordered_set>

namespace dhtoolkit
{
	/*
	 * @brief	Implementation that randomly chooses a vector and performs a double sweep. It then keeps the DS result as well as
	 *			distances of up to the max. distance - some constant. It adds all these nodes to a queue and then calculates distances from these nodes,
	 *			keeping those distances as well and adding these vertices to the queue. It stops when it reaches some number of distances (parameter),
	 *			and then calculates the delta on each of the pairs.
	 */
	class LargeDistances : public IGraphAlg
	{
	public:
		LargeDistances();
		virtual ~LargeDistances();

	private:
		/*
		 * @brief	See documentation in IGraphAlg and in the class description above.
		 */
		virtual DeltaHyperbolicity stepImpl();
		virtual void initImpl(const node_combination_t& initialState);
		virtual bool isComplete() const;

		//the minimal number of pairs to collect
		unsigned int _minNumberOfPairs;
		//pairs collected so far
		std::unordered_map<distance_t, std::vector<std::pair<node_index_t, node_index_t> > > _pairsByDistance;
		//number of pairs in pair map
		unsigned int _pairsCollected;
		//the minimal distance to start collecting pairs from
		distance_t _minDistance;
		//distances for each node whose distance vector was calculated
		std::unordered_map<node_index_t, distance_dict_t> _dists;
		//set of nodes to run on next
		std::unordered_set<node_index_t> _nodeSet;
		//set of nodes already processed
		std::unordered_set<node_index_t> _processedNodeSet;

		//this algorithm has no steps - it is either complete or not... when initialized, this boolean is false. After running it is set to true.
		bool _isComplete;
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm();
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace dhtoolkit

#endif //DELTAHYPER_LARGEDISTANCES_H
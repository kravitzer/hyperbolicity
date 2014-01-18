/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_COHEN_H
#define DELTAHYPER_COHEN_H

#include "DeltaHyperbolicityToolkit\IGraphAlg.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include <string>
#include <vector>

namespace dhtoolkit
{
	/*
	 * @brief	Implementation for the Cohen, Coudert, Lancin solution for finding delta hyperbolicity. Calculates the distances for every pair
     *          of vertices, sorts them in decreasing order, and iterates 4-tuples in that order. When a new delta h* is found, all pairs of distance
     *          2h* or less are removed from the collection and are not considered.
	 */
	class Cohen : public IGraphAlg
	{
	public:
		Cohen();
		virtual ~Cohen();
		bool isComplete() const;

	private:
		/*
		 * @brief	See documentation in IGraphAlg and in the class description above.
		 */
		virtual void initImpl(const node_combination_t&);
		virtual DeltaHyperbolicity stepImpl();

        /*
         * @brief   Adds the pair of nodes given to the appropriate place in the pair array.
         */
        void addToArray(const node_index_t origin, const distance_dict_t::const_iterator& it);

		void advancePointers();

        //for each node, holds its distance map
        std::unordered_map<node_index_t, distance_dict_t> _distances;
        //vector where after initialization, element i has the collection of pairs at distance i
        std::vector<std::vector<std::pair<node_index_t, node_index_t>>> _pairs;
        //iterators on the pairs vector, and the end iterator which marks the last element to be considered
        size_t _l1, _l2, _end;
        //the current position inside the pair vector at elements _l1 and _l2
        size_t _l1Pos, _l2Pos;
        //best delta found so far
        delta_t _bestDelta;
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm(const std::string& outputDir);
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace dhtoolkit

#endif //DELTAHYPER_COHEN_H
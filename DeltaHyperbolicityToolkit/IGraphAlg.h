/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_IGRAPHALG_H
#define DELTAHYPER_IGRAPHALG_H

#include "defs.h"
#include "DeltaHyperbolicity.h"

namespace graphs
{
	/*
	 * @brief	This is the general interface for a graph algorithm. When implementing a new algorithm logic,
	 *			one should inherit from this interface and implement its pure virtual methods.
	 */
	class IGraphAlg
	{
	public:
		/*
		 * @brief	empty ctor.
		 */
		IGraphAlg() {};

		/*
		 * @brief	The implementation of this method should containg the algorithm logic to be run on the given graph.
		 * @param	graph	The graph to run the algorithm on.
		 * @throws	The implementation may throw standard std::exception upon error.
		 */
		virtual DeltaHyperbolicity run(const graph_ptr_t graph) = 0;

		/*
		 * @brief	The implementation of this method should containg the algorithm logic to be run on the given graph.
		 * @param	graph			The graph to run the algorithm on.
		 * @param	initialState	Some initial state for the algorithm to start from.
		 * @throws	The implementation may throw standard std::exception upon error.
		 */
		virtual DeltaHyperbolicity runWithInitialState(const graph_ptr_t graph, const node_quad_t& initialState) = 0;
	};

} // namespace graphs

#endif
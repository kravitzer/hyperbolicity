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
		 * @brief	Empty ctor.
		 */
		IGraphAlg();

		/*
		 * @brief	Default dtor.
		 */
		virtual ~IGraphAlg();

		/*
		 * @brief	Runs the implementation for the algorithm, after performing some input validity checks.
		 * @throws	std::exception	Upon invalid input.
		 * @see		runImpl
		 */
		DeltaHyperbolicity run(const graph_ptr_t graph);

		/*
		 * @brief	Runs the implementation for the algorithm, after performing some input validity checks.
		 * @throws	std::exception	Upon invalid input.
		 * @see		runWithInitialStateImpl
		 */
		DeltaHyperbolicity runWithInitialState(const graph_ptr_t graph, const node_quad_t& initialState);

	protected:
		/*
		 * @brief	The implementation of this method should containg the algorithm logic to be run on the given graph.
		 * @param	graph	The graph to run the algorithm on.
		 * @throws	The implementation may throw standard std::exception upon error.
		 */
		virtual DeltaHyperbolicity runImpl(const graph_ptr_t graph) = 0;

		/*
		 * @brief	The implementation of this method should containg the algorithm logic to be run on the given graph.
		 * @param	graph			The graph to run the algorithm on.
		 * @param	initialState	Some initial state for the algorithm to start from.
		 * @throws	The implementation may throw standard std::exception upon error.
		 */
		virtual DeltaHyperbolicity runWithInitialStateImpl(const graph_ptr_t graph, const node_quad_t& initialState) = 0;

	private:
		/*
		 * @throws	std::exception	If given graph parameter is null or contains less than a quad of vertices.
		 */
		void validateGraphInput(const graph_ptr_t graph) const;
	};

} // namespace graphs

#endif
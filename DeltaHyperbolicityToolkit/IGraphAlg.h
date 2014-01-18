/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_IGRAPHALG_H
#define DELTAHYPER_IGRAPHALG_H

#include "defs.h"
#include "DeltaHyperbolicity.h"
#include <string>

namespace dhtoolkit
{
	/*
	 * @brief	This is the general interface for a graph algorithm. When implementing a new algorithm logic,
	 *			one should inherit from this interface and implement its pure virtual methods.
	 */
	class IGraphAlg
	{
	public:
		/*
		 * @brief	Default ctor.
		 */
		IGraphAlg();

		/*
		 * @brief	Default dtor.
		 */
		virtual ~IGraphAlg();

		/*
		 * @brief	Calls the derived class' implementation for initializing the run.
		 * @param	graph			The graph to run on.
		 * @param	initialState	A state to start from. May be irrelevant for some algorithms, in which case it is ignored.
		 *							This parameter is optional.
		 */
		void initialize(const graph_ptr_t graph, const node_combination_t& initialState = node_combination_t());

		/*
		 * @brief	Runs a single step of the implementation for the algorithm.
		 * @throws	std::exception	Upon an error.
		 * @returns	The delta & state found in this step (not necessarily the best ones found so far - it is the based class' responsibility to keep the best one).
		 * @see		stepImpl
		 */
		DeltaHyperbolicity step();

		/*
		 * @returns	True iff algorithm run has completed, false otherwise.
		 *			If true, a call to step() will raise an exception.
		 */
		virtual bool isComplete() const = 0;

	protected:
		/*
		 * @brief	Derived implementation should perform initialization steps here.
		 */
		virtual void initImpl(const node_combination_t& initialState) = 0;

		/*
		 * @brief	The implementation of this method should contain the algorithm logic for a single step, to run on the graph.
		 * @throws	The implementation may throw standard std::exception upon error.
		 */
		virtual DeltaHyperbolicity stepImpl() = 0;

		//the graph to run on
		graph_ptr_t _graph;

	private:
		/*
		 * @throws	std::exception	If given graph parameter is null or contains less than a quad of vertices.
		 */
		void validateGraphInput(const graph_ptr_t graph) const;
	};

} // namespace dhtoolkit

#endif
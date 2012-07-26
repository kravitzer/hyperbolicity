/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_SA_H
#define DELTAHYPER_SA_H

#include "SADefs.h"
#include "IGraphAlg.h"

namespace graphs
{

	/*
	 * @brief	An interface for a probability function for the SA process.
	 */
	class ISaProbabilityFunction
	{
	public:
		/*
		 * @param	prevDelta	The delta value of the previous state.
		 * @param	curDelta	The delta value of the new state.
		 * @param	curTemp		The current temperature value.
		 * @returns				The probability to accept the new state (number b/w 0.0 to 1.0).
		 */
		virtual sa_probability_t ProbabilityToAcceptChange(delta_t prevDelta, delta_t curDelta, sa_temp_t curTemp) = 0;
	};

	/*
	 * @brief	An interface for a temperature policy function for the SA process.
	 */
	class ISaTempPolicyFunction
	{
	public:
		/*
		 * @param	curTemp		The current temperature.
		 * @param	prevDelta	The delta value of the previous state.
		 * @param	curDelta	The delta value of the new state.
		 * @returns				The new temperature value to be set.
		 */
		virtual sa_temp_t TemperatureChange(sa_temp_t curTemp, delta_t prevDelta, delta_t curDelta) = 0;
	};

	/*
	 * @brief	An implementation for the simulated annealing algorithm.
	 */
	class SimulatedAnnealing : public IGraphAlg
	{
	public:
		/*
		 * @brief	Ctor, initializes the SA process.
		 * @param	initialTemp				The initial temperature.
		 * @param	probabilityFunction		The probability method to be used in the process.
		 * @param	tempFunction			The temperature method to be used in the process.
		 */
		SimulatedAnnealing(sa_temp_t initialTemp, ISaProbabilityFunction& probabilityFunction, ISaTempPolicyFunction& tempFunction);

	private:
		//do *not* allow copy ctor / assignment operator
		SimulatedAnnealing(const SimulatedAnnealing& other);
		SimulatedAnnealing& operator=(const SimulatedAnnealing& other);

		/*
		 * @brief	Implementation of the base class' method. Implements a delta hyperbolicity process on the graph (random initial state).
		 */
		virtual DeltaHyperbolicity runImpl(const graph_ptr_t graph);

		/*
		 * @brief	Implementation of the base class' method. Implements a delta hyperbolicity process on the graph, starting from the given state.
		 */
		virtual DeltaHyperbolicity runWithInitialStateImpl(const graph_ptr_t graph, const node_quad_t& initialState);

		/*
		 * @brief	Starts the SA process.
		 * @param	graph			The graph to run on.
		 * @param	initialState	The initial state of the SA process.
		 * @returns	The result of the SA process (i.e. best delta found in the process).
		 */
		DeltaHyperbolicity startSA(const graph_ptr_t graph, const node_quad_t& initialState);

		/*
		 * @brief	Calculates a "neighboring" state to the current state. I.e. selects a random neighbor of one of the nodes in the quad (also selected
		 *			at random).
		 * @param	graph			The graph to run on.
		 * @param	curState		The current state for whom we find a neighboring state.
		 * @param	This parameter is set to a new state that is one step away from the current state.
		 */
		void step(const graph_ptr_t graph, const node_quad_t& curState, node_quad_t* newState) const;

		//SA parameters (see ctor for details)
		sa_temp_t _temp;
		ISaProbabilityFunction& _probFunc;
		ISaTempPolicyFunction& _tempFunc;
	};

} // namespace graphs

#endif // DELTAHYPER_SA_H
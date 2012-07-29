/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_SA_H
#define DELTAHYPER_SA_H

#include "SADefs.h"
#include "IGraphAlg.h"

namespace dhtoolkit
{

	/*
	 * @brief	An interface for a probability function for the SA process.
	 */
	class ISaProbabilityFunction
	{
	public:
		/*
		 * @param	curDelta	The delta value of the current state.
		 * @param	newDelta	The delta value of the new state.
		 * @param	curTemp		The current temperature value.
		 * @returns				The probability to accept the new state (number b/w 0.0 to 1.0).
		 */
		virtual sa_probability_t ProbabilityToAcceptChange(delta_t curDelta, delta_t newDelta, sa_temp_t curTemp) = 0;

		/*
		 * @brief	Resets any internal state the instance might have, in order to start over when necessary.
		 */
		virtual void reset() = 0;
	};

	/*
	 * @brief	An interface for a temperature policy function for the SA process.
	 */
	class ISaTempPolicyFunction
	{
	public:
		/*
		 * @param	curTemp		The current temperature.
		 * @param	curDelta	The delta value of the current state.
		 * @param	newDelta	The delta value of the new state.
		 * @returns				The new temperature value to be set.
		 */
		virtual sa_temp_t TemperatureChange(sa_temp_t curTemp, delta_t curDelta, delta_t newDelta) = 0;

		/*
		 * @returns	The initial temperature of the algorithm.
		 */
		virtual sa_temp_t GetInitialTemperature() const = 0;

		/*
		 * @brief	Resets any internal state the instance might have, in order to start over when necessary.
		 */
		virtual void reset() = 0;
	};

	/*
	 * @brief	An implementation for the simulated annealing algorithm.
	 */
	class SimulatedAnnealing : public IGraphAlg
	{
	public:
		/*
		 * @brief	Ctor, initializes the SA process.
		 * @param	probabilityFunction		The probability method to be used in the process.
		 * @param	tempFunction			The temperature method to be used in the process.
		 */
		SimulatedAnnealing(sa_prob_func_ptr probabilityFunction, sa_temp_func_ptr tempFunction);

		/*
		 * @brief	Default dtor.
		 */
		virtual ~SimulatedAnnealing();

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
		sa_prob_func_ptr _probFunc;
		sa_temp_func_ptr _tempFunc;
	};

} // namespace dhtoolkit

#endif // DELTAHYPER_SA_H
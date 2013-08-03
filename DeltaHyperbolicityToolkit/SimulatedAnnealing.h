/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_SA_H
#define DELTAHYPER_SA_H

#include "SADefs.h"
#include "IGraphAlg.h"
#include <string>

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
	 * @brief	An interface for a callback method that is called on every step of the SA process.
	 */
	class ISaCallbackFunction
	{
	public:
		/*
		 * @param	graph				The graph the SA process is running on.
		 * @param	currentState		The current state in the SA process.
		 * @param	currentDelta		The delta value of the current state.
		 * @param	currentTemperature	The current temperature of the SA process.
		 * @param	isFinal				When true - this is the last call to the callback function (i.e. process has ended). The values passed are not
		 *								the "current", but the optimal ones observed in the process.
		 */
		virtual void callback(const graph_ptr_t graph, const node_quad_t& currentState, delta_t currentDelta, sa_temp_t currentTemperature, bool isFinal) = 0;

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
		 * @param	outputDir				The dir into which outputs are to be written.
		 * @param	probabilityFunction		The probability method to be used in the process.
		 * @param	tempFunction			The temperature method to be used in the process.
		 * @param	callbackFunction		The callback method to be called on every step of the SA (may be null).
		 */
		SimulatedAnnealing(const std::string& outputDir, sa_prob_func_ptr probabilityFunction, sa_temp_func_ptr tempFunction, sa_callback_func_ptr callbackFunction);

		/*
		 * @brief	Default dtor.
		 */
		virtual ~SimulatedAnnealing();

		/*
		 * @returns	True if temperature is <= 0, false otherwise.
		 */
		virtual bool isComplete() const;

	private:
		//do *not* allow copy ctor / assignment operator
		SimulatedAnnealing(const SimulatedAnnealing& other);
		SimulatedAnnealing& operator=(const SimulatedAnnealing& other);

		/*
		 * @brief	Implementation of the base class' method. Implements a delta hyperbolicity process on the graph (random initial state).
		 */
		virtual DeltaHyperbolicity stepImpl();

		/*
		 * @brief	Resets any internal state the instance might have, in order to run the algorithm again from scratch.
		 */
		virtual void initImpl(const node_quad_t& initialState);

		/*
		 * @brief	Calculates the 6 distances for the current state.
		 * @returns	The delta for the current state, based on those 6 distances.
		 */
		delta_t calculateCurrentDelta();

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
         * @returns The index of the node in the state structure that changed (NOTE: not the index of the node,
         *          but the index in the state collection! e.g. b/w 0 to 3!!).
		 */
		unsigned int getNeighbor(const graph_ptr_t graph, const node_quad_t& curState, node_quad_t* newState) const;

		//current state & delta
		node_quad_t _curState;
		delta_t _curDelta;

		//is this the first step after initialization
		bool _isFirstStep;

		//collection to be used when calculating distances to some specific nodes
		node_ptr_collection_t _destinationNodes;
		//the 6 distances needed to calculate delta (v1->v2, v1->v3, v1->v4, v2->v3, v2->v4, v3->v4, in that order!)
		distance_t _nodeDistances[6];

		//SA parameters (see ctor for details)
		sa_temp_t _temp;
		sa_prob_func_ptr _probFunc;
		sa_temp_func_ptr _tempFunc;
		sa_callback_func_ptr _callbackFunc;
	};

} // namespace dhtoolkit

#endif // DELTAHYPER_SA_H
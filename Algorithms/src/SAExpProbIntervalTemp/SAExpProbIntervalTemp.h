/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_SA_EXP_PROB_INTERVAL_TEMP_H
#define DELTAHYPER_SA_EXP_PROB_INTERVAL_TEMP_H

#include "SimulatedAnnealing.h"
#include "SADefs.h"

namespace dhtoolkit
{
	/*
	 * @brief	Implementation for a simulated annealing probability function that is exponential in the delta-difference
	 *			over the temperature value.
	 */
	class ExpProbability : public ISaProbabilityFunction
	{
	public:
		/*
		 * @brief	Default ctor.
		 */
		ExpProbability();
		virtual ~ExpProbability();

		//allow default ctor, copy ctor & assignment operator
		//ExpProbability(const ExpProbability& other);
		//ExpProbability& operator=(const ExpProbability& other);

		/*
		 * @param	curDelta	The current delta value.
		 * @param	newDelta	The new delta value to be accepted/rejected.
		 * @param	curTemp		The current temperature.
		 * @returns	The probability to accept the new delta
		 */
		sa_probability_t ProbabilityToAcceptChange(delta_t curDelta, delta_t newDelta, sa_temp_t curTemp);

		/*
		 * @brief	Resets any internal state the instance might have, in order to start over when necessary.
		 */
		void reset();
	};


	/*
	 * @brief	Implementation for a simulated annealing temperature function that halves the temperature whenever the delta increases,
	 *			after at least a constant number of iterations.
	 */
	class IntervalTemperature : public ISaTempPolicyFunction
	{
	public:
		/*
		 * @brief	Default ctor.
		 */
		IntervalTemperature();
		virtual ~IntervalTemperature();

		//allow copy ctor & assignment operator
		//IntervalTemperature(const IntervalTemperature& other);
		//IntervalTemperature& operator=(const IntervalTemperature& other);

		/*
		 * @param	curTemp		The current temperature.
		 * @param	curDelta	The current delta value.
		 * @param	newDelta	The new delta value.
		 * @returns	The new temperature to be set.
		 */
		sa_temp_t TemperatureChange(sa_temp_t curTemp, delta_t curDelta, delta_t newDelta);

		/*
		 * @returns	The initial temperature of the algorithm.
		 */
		sa_temp_t GetInitialTemperature() const;

		/*
		 * @brief	Resets any internal state the instance might have, in order to start over when necessary.
		 */
		void reset();

	private:
		static const unsigned int IterationInterval = 25;
		static const sa_temp_t InitialTemp;
		static const sa_temp_t TerminationThreshold;

		//number of times the temperature function has been called
		unsigned int _numOfIterations;

		//the last time (in iteration counts) the temperature value has been updated
		unsigned int _lastChange;
	};

	/*
	 * @brief	Callback used to draw the graph (and its current state) once every so many steps.
	 */
	class DrawingCallback : public ISaCallbackFunction
	{
	public:
		/*
		 * @param	outputDir		The folder into which drawing files will be created.
		 */
		DrawingCallback(const std::string& outputDir);
		virtual ~DrawingCallback();

		/*
		 * @brief	Once every DrawingInterval times, draws th graph into a file whose name is:
		 *			<graph_title>_<run-number>_<step-count>_<delta>_<temperature>[_final].net
		 *			where the "_final" appears only when isFinal is true.
		 * @param	graph				The graph to be drawn.
		 * @param	currentState		The current state - its nodes will be colored.
		 * @param	currentDelta		The delta of the current state - will be indicated in the drawing's title.
		 * @param	currentTemperature	The SA process' current temperature (will appear in file name).
		 * @param	isFinal				When this flag is on - this is the final step of the SA process. File name will end with "_final".
		 */
		void callback(const graph_ptr_t graph, const node_combination_t& currentState, delta_t currentDelta, sa_temp_t currentTemperature, bool isFinal);

		/*
		 * @brief	Resets any internal state the instance might have, in order to start over when necessary.
		 */
		void reset();

	private:
		static const unsigned int DrawingInterval = 100;

		//the directory into which drawing files will be created
		std::string _outputDir;
		//number of times the callback was called
		unsigned int _stepCount;
		//every call to reset increments this count by 1
		unsigned int _runCount;
	};


	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm();
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace dhtoolkit

#endif // DELTAHYPER_SA_EXP_PROB_INTERVAL_TEMP_H
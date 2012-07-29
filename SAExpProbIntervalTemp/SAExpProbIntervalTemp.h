/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_SA_EXP_PROB_INTERVAL_TEMP_H
#define DELTAHYPER_SA_EXP_PROB_INTERVAL_TEMP_H

#include "DeltaHyperbolicityToolkit\SimulatedAnnealing.h"
#include "DeltaHyperbolicityToolkit\SADefs.h"

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


	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm();
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace dhtoolkit

#endif // DELTAHYPER_SA_EXP_PROB_INTERVAL_TEMP_H
#include "SAExpProbIntervalTemp.h"
#include "DeltaHyperbolicityToolkit\SADefs.h"
#include "DeltaHyperbolicityToolkit\SimulatedAnnealing.h"
#include "DeltaHyperbolicityToolkit\IGraphAlg.h"
#include <math.h>
#include <time.h>

namespace dhtoolkit
{
	ExpProbability::ExpProbability()
	{
		reset();
	}

	sa_probability_t ExpProbability::ProbabilityToAcceptChange(delta_t curDelta, delta_t newDelta, sa_temp_t curTemp)
	{
		if (newDelta > curDelta) return 1.0;
		return exp((newDelta-curDelta)/curTemp);
	}

	void ExpProbability::reset()
	{
		//empty
	}

	const sa_temp_t IntervalTemperature::InitialTemp = 100.0;
	const sa_temp_t IntervalTemperature::TerminationThreshold = 0.05;

	IntervalTemperature::IntervalTemperature()
	{
		reset();
	}

	sa_temp_t IntervalTemperature::TemperatureChange(sa_temp_t curTemp, delta_t curDelta, delta_t newDelta)
	{
		++_numOfIterations;

		if ( (newDelta > curDelta) && (_numOfIterations - _lastChange > IterationInterval) )
		{
			_lastChange = _numOfIterations;
			return curTemp/2;
		}

		if (curTemp < TerminationThreshold)
		{
			_lastChange = _numOfIterations;
			return 0;
		}

		return curTemp;
	}

	sa_temp_t IntervalTemperature::GetInitialTemperature() const 
	{
		return InitialTemp;
	}

	void IntervalTemperature::reset()
	{
		_numOfIterations = 0;
		_lastChange = 0;
	}


	IGraphAlg* CreateAlgorithm()
	{
		//initialize random seed (necessary before calling DoubleSweep() ).
		srand( static_cast<unsigned int>(time(nullptr)) );

		sa_prob_func_ptr probFunc(new ExpProbability);
		sa_temp_func_ptr tempFunc(new IntervalTemperature);
		IGraphAlg* alg = new SimulatedAnnealing(probFunc, tempFunc);
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
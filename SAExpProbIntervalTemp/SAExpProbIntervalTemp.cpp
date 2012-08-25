#include "SAExpProbIntervalTemp.h"
#include "DeltaHyperbolicityToolkit\SADefs.h"
#include "DeltaHyperbolicityToolkit\SimulatedAnnealing.h"
#include "DeltaHyperbolicityToolkit\IGraphAlg.h"
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include "boost\format.hpp"
#include <math.h>
#include <time.h>
#include <string>
#include <algorithm>

using namespace std;

const char SlashReplacement = '~';
const string DrawingExtension = "net";

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


	DrawingCallback::DrawingCallback(const string& outputDir) : _runCount(-1), _outputDir(outputDir)
	{
		if ( (_outputDir.size() > 0) && (_outputDir[_outputDir.size()-1] != '\\') ) _outputDir += '\\';
		reset();
	}

	void DrawingCallback::callback(const graph_ptr_t graph, const node_quad_t& currentState, delta_t currentDelta, sa_temp_t currentTemperature, bool isFinal)
	{
		if ( (_stepCount % DrawingInterval == 0) || (isFinal) )
		{
			//construct file name
			string graphName = graph->getTitle();
			replace(graphName.begin(), graphName.end(), '\\', SlashReplacement);

			string fileName;
			if (!isFinal)
			{
				fileName = (boost::format("%1%%2%_%3%_%4%_%5%_%6%.%7%") % _outputDir % graphName % _runCount % _stepCount % currentDelta % currentTemperature % DrawingExtension).str();
			}
			else
			{
				fileName = (boost::format("%1%%2%_%3%_%4%_%5%_%6%_final.%7%") % _outputDir % graphName % _runCount % _stepCount % currentDelta % currentTemperature % DrawingExtension).str();
			}
		
			//draw graph
			GraphAlgorithms::drawGraph(fileName, graph, &currentState);
		}

		//increment step count
		++_stepCount;
	}

	void DrawingCallback::reset()
	{
		_stepCount = 0;
		++_runCount;
	}


	IGraphAlg* CreateAlgorithm(const string& outputDir)
	{
		//initialize random seed (necessary before calling DoubleSweep() ).
		srand( static_cast<unsigned int>(time(nullptr)) );

		sa_prob_func_ptr probFunc(new ExpProbability);
		sa_temp_func_ptr tempFunc(new IntervalTemperature);
		sa_callback_func_ptr callbackFunc(new DrawingCallback(outputDir));
		IGraphAlg* alg = new SimulatedAnnealing(outputDir, probFunc, tempFunc, callbackFunc);
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
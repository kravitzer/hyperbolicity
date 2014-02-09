#include "MDS.h"
#include "Graph\defs.h"
#include "Algorithm\DeltaHyperbolicity.h"
#include "Graph\GraphAlgorithms.h"
#include "Graph\NodeDistances.h"
#include <time.h>
#include <string>
#include <vector>
#include <math.h>

using namespace std;

namespace dhtoolkit
{
	MDS::MDS() : IGraphAlg(), _isCompleted(false)
	{
		//empty
	}

	MDS::~MDS()
	{
		//empty
	}

	DeltaHyperbolicity MDS::stepImpl()
	{	
		HyperbolicityAlgorithms::DoubleSweepResult curDS = _sweeps[_sweeps.size()-1];
		delta_t maxDelta = 0;
		node_combination_t maxState;
		for (unsigned int i = 0; i < _sweeps.size()-1; ++i)
		{
			delta_t curDelta = 0;
			//if pair has a node in common, or if sweeps are in different strongly connected components - the delta is considered to be 0
			//therefore we only calculate delta if none of these conditions is not met
			if ( (areSweepsUnique(_sweeps[i], curDS)) && (_sweeps[i].uDistances[curDS.u->getIndex()] != InfiniteDistance) )
			{
				//calculate delta by using the existing distances
				distance_t d1 = _sweeps[i].dist + curDS.dist;
				distance_t d2 = _sweeps[i].uDistances[curDS.u->getIndex()] + _vDists[i][curDS.v->getIndex()];
				distance_t d3 = _sweeps[i].uDistances[curDS.v->getIndex()] + _vDists[i][curDS.u->getIndex()];
				curDelta = HyperbolicityAlgorithms::calculateDeltaFromDistances(d1, d2, d3);
			}
				
			//if we've reached a better delta value, keep it
			if ( (curDelta > maxDelta) || (!maxState.isInitialized()) )
			{
				maxDelta = curDelta;
				maxState.reset(_sweeps[i].u, _sweeps[i].v, curDS.u, curDS.v);
			}
		}

		prepareNextStep();
		
		return DeltaHyperbolicity(maxDelta, maxState);
	}
	
	void MDS::prepareNextStep()
	{
		HyperbolicityAlgorithms::DoubleSweepResult curDS = HyperbolicityAlgorithms::doubleSweep(_graph);
		unsigned int numOfTrials = 1;
		for (; numOfTrials < MaxNumOfTrials && !isNewSweep(curDS); ++numOfTrials)
		{
			//calculate another double-sweep
			curDS = HyperbolicityAlgorithms::doubleSweep(_graph);
		}

		//if couldn't find another double-sweep before exceeding maximal number of trials, we're done
		if (!isNewSweep(curDS))
		{
			_isCompleted = true;
			return;
		}

		_sweeps.push_back(curDS);
		//calculate all v-distances from previous-to-last double-sweep
		_vDists.push_back( NodeDistances(_graph, _sweeps[_sweeps.size()-2].v).getDistances() );
	}

	bool MDS::areSweepsUnique(HyperbolicityAlgorithms::DoubleSweepResult& res1, HyperbolicityAlgorithms::DoubleSweepResult& res2)
	{
		return ( (res1.u != res2.u) && (res1.v != res2.u) && (res1.u != res2.v) && (res1.v != res2.v) && (res1.u != res1.v) && (res2.u != res2.v) );
	}

	bool MDS::isNewSweep(const HyperbolicityAlgorithms::DoubleSweepResult& ds) const
	{
		for (vector<HyperbolicityAlgorithms::DoubleSweepResult>::const_iterator it = _sweeps.cbegin(); it != _sweeps.cend(); ++it)
		{
			if ( (it->u == ds.u && it->v == ds.v) || (it->u == ds.v && it->v == ds.u) ) return false;
		}

		return true;
	}

	void MDS::initImpl(const node_combination_t&)
	{
		_isCompleted = false;
		_vDists.clear();
		_sweeps.clear();

		//we need at least one double-sweep before each step
		_sweeps.push_back(HyperbolicityAlgorithms::doubleSweep(_graph));
		prepareNextStep();
	}

	bool MDS::isComplete() const
	{
		//we can always run one more...
		return _isCompleted;
	}



	IGraphAlg* CreateAlgorithm()
	{
		//initialize random seed (necessary before calling doubleSweep() )
		srand(static_cast<unsigned int>(time(nullptr)));

		IGraphAlg* alg = new MDS();
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
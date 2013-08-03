#include "MDSweep.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include "DeltaHyperbolicityToolkit\NodeDistances.h"
#include <time.h>
#include <string>
#include <vector>
#include <math.h>

using namespace std;

const unsigned int MinNumOfSweeps = 100;

namespace dhtoolkit
{
	MDSweep::MDSweep(const string& outputDir) : IGraphAlg(outputDir) 
	{
		//empty
	}

	MDSweep::~MDSweep()
	{
		//empty
	}

	DeltaHyperbolicity MDSweep::stepImpl()
	{	
		GraphAlgorithms::DoubleSweepResult curDS = GraphAlgorithms::DoubleSweep(_graph);
		unsigned int numOfTrials = 1;
		for (; numOfTrials < MaxNumOfTrials && !isNewSweep(curDS); ++numOfTrials)
		{
			//calculate another double-sweep
			curDS = GraphAlgorithms::DoubleSweep(_graph);
		}
		if (!isNewSweep(curDS)) throw std::exception("Exceeded the maximal number of trials allowed!");

		_sweeps.push_back(curDS);
		//calculate all v-distances from previous-to-last double-sweep
		_vDists.push_back( NodeDistances(_graph, _sweeps[_sweeps.size()-2].v).getDistances() );

		delta_t maxDelta = 0;
		node_quad_t maxState;
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
				curDelta = GraphAlgorithms::CalculateDeltaFromDistances(d1, d2, d3);
			}
				
			//if we've reached a better delta value, keep it
			if ( (curDelta > maxDelta) || (!maxState.isInitialized()) )
			{
				maxDelta = curDelta;
				maxState.reset(_sweeps[i].u, _sweeps[i].v, curDS.u, curDS.v);
			}
		}
		
		return DeltaHyperbolicity(maxDelta, maxState);
	}
	
	bool MDSweep::areSweepsUnique(GraphAlgorithms::DoubleSweepResult& res1, GraphAlgorithms::DoubleSweepResult& res2)
	{
		return ( (res1.u != res2.u) && (res1.v != res2.u) && (res1.u != res2.v) && (res1.v != res2.v) && (res1.u != res1.v) && (res2.u != res2.v) );
	}

	bool MDSweep::isNewSweep(const GraphAlgorithms::DoubleSweepResult& ds) const
	{
		for (vector<GraphAlgorithms::DoubleSweepResult>::const_iterator it = _sweeps.cbegin(); it != _sweeps.cend(); ++it)
		{
			if ( (it->u == ds.u && it->v == ds.v) || (it->u == ds.v && it->v == ds.u) ) return false;
		}

		return true;
	}

	void MDSweep::initImpl(const node_quad_t&)
	{
		_vDists.clear();
		_sweeps.clear();

		//we need at least one double-sweep before each step
		_sweeps.push_back(GraphAlgorithms::DoubleSweep(_graph));
	}

	bool MDSweep::isComplete() const
	{
		//we can always run one more...
		return false;
	}



	IGraphAlg* CreateAlgorithm(const string& outputDir)
	{
		//initialize random seed (necessary before calling DoubleSweep() )
		srand(static_cast<unsigned int>(time(nullptr)));

		IGraphAlg* alg = new MDSweep(outputDir);
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
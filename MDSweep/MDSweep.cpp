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

	DeltaHyperbolicity MDSweep::runImpl(const graph_ptr_t graph) 
	{
		unsigned int numOfSweeps = numOfDoubleSweeps(graph->size());
		vector<GraphAlgorithms::DoubleSweepResult> sweeps;
		
		//calculate a bunch of double sweeps
		for (unsigned int i = 0; i < numOfSweeps; ++i)
		{
			sweeps.push_back(GraphAlgorithms::DoubleSweep(graph));
		}
		
		
		delta_t maxDelta = 0;
		node_quad_t maxState;
		
		//check all pair-combinations of the double sweeps collected, keep the best delta received
		for (unsigned int i = 0; i < numOfSweeps - 1; ++i)
		{
			distance_dict_t vDists = NodeDistances(graph, sweeps[i].v).getDistances();
			for (unsigned int j = i+1; j < numOfSweeps; ++j)
			{
				//skip this pair if it has a node in common
				if (!areSweepsUnique(sweeps[i], sweeps[j])) continue;
                //skip this pair if sweeps are not in strongly connected components (i.e. distance from a node in one sweep is infinite from a node in another)
                if (sweeps[i].uDistances[sweeps[j].u->getIndex()] == InfiniteDistance) continue;
				
				//calculate delta by using the existing distances
				distance_t d1 = sweeps[i].dist + sweeps[j].dist;
				distance_t d2 = sweeps[i].uDistances[sweeps[j].u->getIndex()] + vDists[sweeps[j].v->getIndex()];
				distance_t d3 = sweeps[i].uDistances[sweeps[j].v->getIndex()] + vDists[sweeps[j].u->getIndex()];
				delta_t curDelta = GraphAlgorithms::CalculateDeltaFromDistances(d1, d2, d3);
				
				//if we've reached a better delta value, keep it
				if (curDelta > maxDelta)
				{
					maxDelta = curDelta;
					maxState.reset(sweeps[i].u, sweeps[i].v, sweeps[j].u, sweeps[j].v);
				}
			}
		}
		
		return DeltaHyperbolicity(maxDelta, maxState);
	}

	DeltaHyperbolicity MDSweep::runWithInitialStateImpl(const graph_ptr_t graph, const node_quad_t& initialState)
	{
		return runAndReturnBetter(graph, initialState);
	}
	
	unsigned int MDSweep::numOfDoubleSweeps(unsigned int numOfNodes) const
	{
		unsigned int count = static_cast<unsigned int>(sqrt(static_cast<float>(numOfNodes)));
		return (count > MinNumOfSweeps) ? count : MinNumOfSweeps;
	}
	
	bool MDSweep::areSweepsUnique(GraphAlgorithms::DoubleSweepResult& res1, GraphAlgorithms::DoubleSweepResult& res2)
	{
		return ( (res1.u != res2.u) && (res1.v != res2.u) && (res1.u != res2.v) && (res1.v != res2.v) && (res1.u != res1.v) && (res2.u != res2.v) );
	}

	void MDSweep::reset()
	{
		//empty
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
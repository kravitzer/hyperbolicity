#include "DDS.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include "DeltaHyperbolicityToolkit\NodeDistances.h"
#include <time.h>
#include <string>

using namespace std;

namespace dhtoolkit
{
	DDS::DDS() : IGraphAlg() 
	{
		//empty
	}

	DDS::~DDS()
	{
		//empty
	}

	DeltaHyperbolicity DDS::stepImpl() 
	{
		GraphAlgorithms::DoubleSweepResult ds1 = GraphAlgorithms::DoubleSweep(_graph);
		GraphAlgorithms::DoubleSweepResult ds2 = GraphAlgorithms::DoubleSweep(_graph);
		
		unsigned int numOfTrials = 1;

		while ( (ds1.u == ds2.u) || (ds1.v == ds2.u) || (ds1.u == ds2.v) || (ds1.v == ds2.v) || (ds1.u == ds1.v) || (ds2.u == ds2.v) )
		{
			if (numOfTrials >= MaxNumOfTrials) throw std::exception("Exceeded the maximal number of trials allowed!");
			ds2 = GraphAlgorithms::DoubleSweep(_graph);
			++numOfTrials;
		}

		node_combination_t state(ds1.u, ds1.v, ds2.u, ds2.v);
		//at this point we have the distances from ds1.u and ds2.u to all other nodes in the graph
		//all we need to calculate the delta is the distance from ds1.v to ds2.v
		NodeDistances distanceFromDs1v(_graph, ds1.v);
		distance_t ds1vToDs2v = distanceFromDs1v.getDistance(ds2.v);

		//calculate the sum of distances of the three matchings for the 4 vertices
		distance_t d1 = ds1.dist + ds2.dist;
		distance_t d2 = ds1.uDistances[ds2.u->getIndex()] + ds1vToDs2v;
		distance_t d3 = ds1.uDistances[ds2.v->getIndex()] + ds2.uDistances[ds1.v->getIndex()];

		delta_t delta = GraphAlgorithms::CalculateDeltaFromDistances(d1, d2, d3);
		return DeltaHyperbolicity(delta, state);
	}

	void DDS::initImpl(const node_combination_t&)
	{
		//empty
	}

	bool DDS::isComplete() const
	{
		//we can always run one more...
		return false;
	}



	IGraphAlg* CreateAlgorithm()
	{
		//initialize random seed (necessary before calling DoubleSweep() )
		srand(static_cast<unsigned int>(time(nullptr)));

		IGraphAlg* alg = new DDS();
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
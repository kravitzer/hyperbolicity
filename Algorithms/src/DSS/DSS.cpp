#include "DSS.h"
#include "Graph\defs.h"
#include "Graph\DeltaHyperbolicity.h"
#include "Graph\GraphAlgorithms.h"
#include "Graph\NodeDistances.h"
#include <time.h>
#include <string>

using namespace std;

namespace dhtoolkit
{
    const distance_t MaxDistance = INT_MAX;

	DSS::DSS() : IDSweepMinExt(), _minDistance(MaxDistance)
	{
		//empty
	}

	DSS::~DSS()
	{
		//empty
	}

	void DSS::initStep()
	{
		_v3Candidates.clear();
		_minDistance = MaxDistance;
	}

	void DSS::processV3Candidate(node_ptr_t v3Candidate, distance_t distFromV1, distance_t distFromV2)
	{
		if ( (distFromV1 <= distFromV2 && distFromV2 <= distFromV1 + 1) || (distFromV2 <= distFromV1 && distFromV1 <= distFromV2 + 1) )
		{
			if (distFromV1 + distFromV2 <=  _minDistance)
			{
				if (distFromV1 + distFromV2 <  _minDistance)
				{
					_minDistance = distFromV1 + distFromV2;
					_v3Candidates.clear();						
				}
				_v3Candidates.push_back(v3Candidate);
			}
		}
	}






	IGraphAlg* CreateAlgorithm()
	{
		//initialize random seed (necessary before calling DoubleSweep() ).
		srand(static_cast<unsigned int>(time(nullptr)));

		IGraphAlg* alg = new DSS();
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
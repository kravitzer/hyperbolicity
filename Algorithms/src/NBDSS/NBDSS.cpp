#include "NBDSS.h"
#include "DeltaHyperbolicityToolkit\IGraphAlg.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include <string>
#include <time.h>

using namespace std;

namespace dhtoolkit
{
    const distance_t MaxDistance = INT_MAX;

	NBDSS::NBDSS() : IDSweepMinExt()
	{
		//empty
	}

	NBDSS::~NBDSS()
	{
		//empty
	}

	void NBDSS::initStep()
	{
		_v3Candidates.clear();
		_minDistance = MaxDistance;
	}

	void NBDSS::processV3Candidate(node_ptr_t v3Candidate, distance_t distFromV1, distance_t distFromV2)
	{
		if (distFromV1 + distFromV2 <= _minDistance)
		{
			if (distFromV1 + distFromV2 < _minDistance)
			{
				_minDistance = distFromV1 + distFromV2;
				_v3Candidates.clear();						
			}
			_v3Candidates.push_back(v3Candidate);
		}
	}






	IGraphAlg* CreateAlgorithm()
	{
		//initialize random seed (necessary before calling DoubleSweep() ).
		srand(static_cast<unsigned int>(time(nullptr)));

		IGraphAlg* alg = new NBDSS();
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
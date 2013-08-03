#include "DSweepMin.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include "DeltaHyperbolicityToolkit\NodeDistances.h"
#include <time.h>
#include <string>

using namespace std;

namespace dhtoolkit
{
    const distance_t MaxDistance = INT_MAX;

	DSweepMin::DSweepMin(const string& outputDir) : IDSweepMinExt(outputDir), _minDistance(MaxDistance)
	{
		//empty
	}

	DSweepMin::~DSweepMin()
	{
		//empty
	}

	void DSweepMin::initStep()
	{
		_v3Candidates.clear();
		_minDistance = MaxDistance;
	}

	void DSweepMin::processV3Candidate(node_ptr_t v3Candidate, distance_t distFromV1, distance_t distFromV2)
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






	IGraphAlg* CreateAlgorithm(const string& outputDir)
	{
		//initialize random seed (necessary before calling DoubleSweep() ).
		srand(static_cast<unsigned int>(time(nullptr)));

		IGraphAlg* alg = new DSweepMin(outputDir);
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
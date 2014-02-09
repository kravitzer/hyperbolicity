/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#pragma once

#include "Algorithm\DeltaHyperbolicity.h"
#include "Algorithm\IGraphAlg.h"
#include "IDSS.h"
#include <string>

namespace hyperbolicity
{
	/*
	 * @brief	Implementation for the IDSweepMinExt algorithm with v3 candidates being the ones that have the minimxal total
	 *			distance from both v1 and v2 (i.e. d(v1, v3) + d(v2, v3) is minimal) AND abs(d(v1, v3) - d(v2, v3)) <= 1 (i.e.
	 *			distances must be balanced).
	 */
	class NBDSS : public IDSweepMinExt
	{
	public:
		NBDSS();
		virtual ~NBDSS();

	private:
		void initStep();
		virtual void processV3Candidate(node_ptr_t v3Candidate, distance_t distFromV1, distance_t distFromV2);
		distance_t _minDistance;
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm();
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace hyperbolicity

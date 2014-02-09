/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#pragma once

#include "Algorithm\IGraphAlg.h"
#include "Algorithm\DeltaHyperbolicity.h"
#include <string>

namespace hyperbolicity
{
	/*
	 * @brief	Implementation for the double-double-sweep algorithm. This algorithm runs a double-sweep on the graph,
	 *			and then repeatedly runs another double-sweep until receiving 4 total different nodes. It then uses these
	 *			4 nodes to calculate the delta hyperbolicity, and returns the result.
	 */
	class DDS : public IGraphAlg
	{
	public:
		DDS();
		virtual ~DDS();

	private:
		/*
		 * @brief	See documentation in IGraphAlg and in the class description above.
		 */
		virtual DeltaHyperbolicity stepImpl();
		virtual void initImpl(const node_combination_t& initialState);
		virtual bool isComplete() const;

		//The maximal number of trials to get 4 different nodes out of separate double-sweeps
		static const unsigned int MaxNumOfTrials = 50;
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm();
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace hyperbolicity

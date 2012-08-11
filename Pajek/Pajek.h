/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_PAJEK_H
#define DELTAHYPER_PAJEK_H

#include "DeltaHyperbolicityToolkit\IGraphAlg.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"

namespace dhtoolkit
{
	/*
	 * @brief	Creates a Pajek net file which you may later load in Pajek to view a visual representation of the graph.
	 */
	class Pajek : public IGraphAlg
	{
	public:
		Pajek() : IGraphAlg() {}

	private:
		/*
		 * @brief	Draws the graph. File name will be graph's title + a fixed file extension.
		 */
		virtual DeltaHyperbolicity runImpl(const graph_ptr_t graph);

		/*
		 * @brief	Draws the graph. Colors the initialState with a different color. File name will be graph's title + the ".net" extension.
		 */
		virtual DeltaHyperbolicity runWithInitialStateImpl(const graph_ptr_t graph, const node_quad_t& initialState);

		/*
		 * @brief	Does nothing as there is no state to reset...
		 */
		virtual void reset();
	};

	extern "C" __declspec(dllexport) IGraphAlg* CreateAlgorithm();
	extern "C" __declspec(dllexport) void ReleaseAlgorithm(IGraphAlg* alg);
} // namespace dhtoolkit

#endif //DELTAHYPER_DSWEEP_H
#ifndef DELTAHYPER_IGRAPHALG_H
#define DELTAHYPER_IGRAPHALG_H

#include "defs.h"

namespace graphs
{

class IGraphAlg
{
public:
	IGraphAlg() {};
	DeltaHyperbolicity run(const graph_ptr_t graph) = 0;
	DeltaHyperbolicity runWithInitialState(const graph_ptr_t graph, const node_quad_t& initialState) = 0;
};

} // namespace graphs

#endif
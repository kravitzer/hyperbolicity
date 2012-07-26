#include "IGraphAlg.h"
#include "defs.h"
#include "Graph.h"

namespace graphs
{

	IGraphAlg::IGraphAlg()
	{
		//empty
	}

	IGraphAlg::~IGraphAlg()
	{
		//empty
	}

	DeltaHyperbolicity IGraphAlg::run(const graph_ptr_t graph)
	{
		validateGraphInput(graph);
		return runImpl(graph);
	}

	DeltaHyperbolicity IGraphAlg::runWithInitialState(const graph_ptr_t graph, const node_quad_t& state)
	{
		validateGraphInput(graph);
		return runWithInitialStateImpl(graph, state);
	}

	void IGraphAlg::validateGraphInput(const graph_ptr_t graph) const
	{
		if (nullptr == graph.get()) throw std::exception("Graph pointer is invalid");
		if (graph->size() < node_quad_t::size()) throw std::exception("Graph does not contain enough nodes");
	}

} // namespace graphs
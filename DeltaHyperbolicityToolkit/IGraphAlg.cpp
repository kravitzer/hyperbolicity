#include "IGraphAlg.h"
#include "defs.h"
#include "Graph.h"
#include "GraphAlgorithms.h"

namespace dhtoolkit
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
		reset();
		return runImpl(graph);
	}

	DeltaHyperbolicity IGraphAlg::runWithInitialState(const graph_ptr_t graph, const node_quad_t& state)
	{
		validateGraphInput(graph);
		reset();
		return runWithInitialStateImpl(graph, state);
	}

	DeltaHyperbolicity IGraphAlg::runAndReturnBetter(const graph_ptr_t graph, const node_quad_t& state)
	{
		//we compute the delta hyperbolicity regardless of the given state, and then simply
		//compare the newly calculated value to the given state's value to make sure we're
		//not returning a result that's worse.

		delta_t givenDelta = GraphAlgorithms::CalculateDelta(graph, state);
		DeltaHyperbolicity newDelta = runImpl(graph);
		if (givenDelta > newDelta.getDelta())
		{
			return DeltaHyperbolicity(givenDelta, state);
		}
		else
		{
			return newDelta;
		}
	}

	void IGraphAlg::validateGraphInput(const graph_ptr_t graph) const
	{
		if (nullptr == graph.get()) throw std::exception("Graph pointer is invalid");
		if (graph->size() < node_quad_t::size()) throw std::exception("Graph does not contain enough nodes");
	}

} // namespace dhtoolkit
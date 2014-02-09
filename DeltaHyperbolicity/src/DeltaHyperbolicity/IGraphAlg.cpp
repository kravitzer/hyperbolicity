#include "IGraphAlg.h"
#include "Graph\defs.h"
#include "Graph\Graph.h"
#include "Graph\GraphAlgorithms.h"
#include <string>

using namespace std;

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

	void IGraphAlg::initialize(const graph_ptr_t graph, const node_combination_t& initialState /* = node_combination_t() */)
	{
		validateGraphInput(graph);
		_graph = graph;

		//run derived class initialization
		initImpl(initialState);
	}

	DeltaHyperbolicity IGraphAlg::step()
	{
		if (isComplete()) throw std::logic_error("Algorithm has already completed its run, cannot perform another step");

		//run a single step
		return stepImpl();
	}

	void IGraphAlg::validateGraphInput(const graph_ptr_t graph) const
	{
		if (nullptr == graph.get()) throw std::exception("Graph pointer is invalid");
		if (graph->size() < node_combination_t::size()) throw std::exception("Graph does not contain enough nodes");
	}

} // namespace dhtoolkit
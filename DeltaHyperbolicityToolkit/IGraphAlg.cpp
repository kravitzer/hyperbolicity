#include "IGraphAlg.h"
#include "defs.h"
#include "Graph.h"
#include "GraphAlgorithms.h"
#include "Except.h"
#include <string>

using namespace std;

namespace dhtoolkit
{

	IGraphAlg::IGraphAlg(const string& outputDir) : _outputDir(outputDir)
	{
		//empty
	}

	IGraphAlg::~IGraphAlg()
	{
		//empty
	}

	void IGraphAlg::initialize(const graph_ptr_t graph, const node_quad_t& initialState /* = node_quad_t() */)
	{
		validateGraphInput(graph);
		_graph = graph;

		//run derived class initialization
		initImpl(initialState);
	}

	DeltaHyperbolicity IGraphAlg::step()
	{
		if (isComplete()) throw AlgorithmCompletedException("Algorithm has already completed its run, cannot perform another step");

		//run a single step
		return stepImpl();
	}

	void IGraphAlg::validateGraphInput(const graph_ptr_t graph) const
	{
		if (nullptr == graph.get()) throw std::exception("Graph pointer is invalid");
		if (graph->size() < node_quad_t::size()) throw std::exception("Graph does not contain enough nodes");
	}

} // namespace dhtoolkit
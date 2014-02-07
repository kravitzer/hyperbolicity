#include "Pajek.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include <memory>
#include <string>
#include <algorithm>

using namespace std;

const string FileExtension = ".net";
const char SlashReplacement = '~';

namespace dhtoolkit
{
	Pajek::Pajek(const string& outputDir) : IGraphAlg(outputDir) 
	{
		//empty
	}

	DeltaHyperbolicity Pajek::runImpl(const graph_ptr_t graph) 
	{
		string fileName = graph->getTitle() + FileExtension;
		//if file name contains slashes - replace them with underscores (so that the file is saved in the local dir rather than someplace else...)
		replace(fileName.begin(), fileName.end(), '\\', SlashReplacement);
		fileName = _outputDir + fileName;

		GraphAlgorithms::drawGraph(fileName, graph, nullptr);
		
		//returns an arbitrary state...
		node_quad_t state(graph->getNode(0), graph->getNode(1), graph->getNode(2), graph->getNode(3));
		delta_t delta = GraphAlgorithms::CalculateDelta(graph, state);
		return DeltaHyperbolicity(delta, state);
	}

	DeltaHyperbolicity Pajek::runWithInitialStateImpl(const graph_ptr_t graph, const node_quad_t& initialState)
	{
		string fileName = graph->getTitle() + FileExtension;
		//if file name contains slashes - replace them with underscores (so that the file is saved in the local dir rather than someplace else...)
		replace(fileName.begin(), fileName.end(), '\\', SlashReplacement);
		fileName = _outputDir + fileName;

		GraphAlgorithms::drawGraph(fileName, graph, &initialState);

		//returns the same state given
		delta_t delta = GraphAlgorithms::CalculateDelta(graph, initialState);
		return DeltaHyperbolicity(delta, initialState);
	}

	void Pajek::reset()
	{
		//empty
	}


	IGraphAlg* CreateAlgorithm(const string& outputDir)
	{
		IGraphAlg* alg = new Pajek(outputDir);
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
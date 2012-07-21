#include <iostream>
#include <Windows.h>
#include "GraphAlgorithms.h"
#include "defs.h"
#include "IGraphAlg.h"

using namespace std;
using namespace graphs;

typedef IGraphAlg* (*AlgCreationMethod)();
typedef void (*AlgReleaseMethod)(IGraphAlg*);

shared_ptr<IGraphAlg> loadAlgorithm(LPTSTR algDllPath)
{
	shared_ptr<HINSTANCE__> alg(LoadLibrary(algDllPath), &CloseHandle);
	if (nullptr == alg.get()) throw std::exception("Failed loading dll");

	AlgCreationMethod createAlg = reinterpret_cast<AlgCreationMethod>(GetProcAddress(alg.get(), "CreateAlgorithm"));
	if (nullptr == createAlg) throw std::exception("Failed to get the algorithm creation method");

	AlgReleaseMethod releaseAlg = reinterpret_cast<AlgReleaseMethod>(GetProcAddress(alg.get(), "ReleaseAlgorithm"));
	if (nullptr == releaseAlg) throw std::exception("Failed to get the algorithm release method");

	return shared_ptr<IGraphAlg>(createAlg(), releaseAlg);
}

DeltaHyperbolicity runAlgorithm(LPTSTR algDllPath, const graph_ptr_t graph)
{
	wcout << TEXT("Loading ") << algDllPath << "..." << endl;
		
	shared_ptr<IGraphAlg> alg = loadAlgorithm(algDllPath);
	return alg->run(graph);
}

DeltaHyperbolicity runAlgorithmWithInitialState(LPTSTR algDllPath, const graph_ptr_t graph, const node_quad_t& state)
{
	cout << "Loading ddsweep dll..." << endl;
		
	shared_ptr<IGraphAlg> alg = loadAlgorithm(algDllPath);
	return alg->runWithInitialState(graph, state);
}

unsigned int countEdges(const graph_ptr_t graph)
{
	unsigned int edgeCount = 0;
	for (unsigned int i = 0; i < graph->size(); ++i)
	{
		edgeCount += graph->getNode(i)->getEdges().size();
	}

	return edgeCount;
}

int main()
{
	try
	{
		graph_ptr_t g = GraphAlgorithms::LoadGraphFromFile("C:\\Users\\Eran\\Dropbox\\University\\Thesis\\Shavitt\\Code\\Graphs\\100-300\\2.dat");
	
		cout << "Graph loaded, " << g->size() << " nodes and " << countEdges(g) << " edges have been loaded!" << endl;
		cout << "Pruning..." << endl;
		GraphAlgorithms::PruneTrees(g);
		cout << "Graph now has " << g->size() << " nodes and " << countEdges(g) << " edges." << endl;

		node_ptr_t origin = g->getNode(0);
		node_collection_t dest(3);
		for (int i = 1; i < 4; ++i) dest[i-1] = g->getNode(i);
		distance_dict_t dist = GraphAlgorithms::Dijkstra(g, g->getNode(0));

		for (distance_dict_t::const_iterator it = dist.cbegin(); it != dist.cend(); ++it)
		{
			if (it->second >= 4) cout << "Node: " << it->first << ", Distance: " << it->second << endl;
		}

		GraphAlgorithms::DoubleSweepResult res = GraphAlgorithms::DoubleSweep(g);
		cout << res.u->getIndex() << "->" << res.v->getIndex() << ", distance: " << res.dist << endl;

		cout << endl << endl;

		cout << runAlgorithm(TEXT("ddsweep.dll"), g).getDelta() << endl;
		node_quad_t quad;
		cout << runAlgorithmWithInitialState(TEXT("ddsweep.dll"), g, quad).getDelta() << endl;
	}
	catch (const std::exception& e)
	{
		cout << "Exception caught: " << e.what() << endl;
	}

	return 0;
}
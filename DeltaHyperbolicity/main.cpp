#include <iostream>
#include "GraphAlgorithms.h"
#include "defs.h"

using namespace std;
using namespace graphs;

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
	}
	catch (const std::exception& e)
	{
		cout << "Exception caught: " << e.what() << endl;
	}

	return 0;
}
#include <Graph\Graph.h>
#include <Graph\GraphAlgorithms.h>
#include <Graph\defs.h>
#include <Graph\FurthestNode.h>
#include <Graph\NodeDistances.h>
#include <Graph\SpanningTree.h>
#include <iostream>
#include <time.h>

using namespace std;
using namespace hyperbolicity;

int main()
{
	try
	{
		srand(static_cast<unsigned int>(time(nullptr)));

		clock_t t1 = clock();
		graph_ptr_t graph = GraphAlgorithms::LoadGraphFromFile("D:\\Eran\\Dropbox\\University\\Thesis\\Shavitt\\Code\\Graphs\\Stanford\\Formatted\\Email-Enron.txt"); //C:\\Users\\Eran\\Dropbox\\University\\Thesis\\Shavitt\\Code\\Graphs\\Stanford\\Formatted\\roadNet-PA.txt");
		double timeElapsed = (clock() - t1) / static_cast<double>(CLOCKS_PER_SEC);
		cout << "Loading took " << timeElapsed << " seconds" << endl;
        cout << "# of vertices: " << graph->size() << endl;
        cout << "# of edges: " << (graph->edgeCount() / 2) << endl << endl;
		
		t1 = clock();
		graph_ptr_collection_t graphs = GraphAlgorithms::getBiconnectedComponents(graph);
		timeElapsed = (clock() - t1) / static_cast<double>(CLOCKS_PER_SEC);
		cout << "Getting biconnected components took " << timeElapsed << " seconds" << endl;
		cout << "# of graphs: " << graphs.size() << endl;
		size_t nodeCount = 0;
		size_t edgeCount = 0;
		unsigned int i = 0;
		for (graph_ptr_collection_t::const_iterator it = graphs.cbegin(); it != graphs.cend(); ++it)
		{
			nodeCount += (*it)->size();
			edgeCount += ((*it)->edgeCount() / 2);
		}

		if (nodeCount < graph->size()) cout << "***ERROR*** - Node count is smaller than the sum of nodes in the biconnected components (" << nodeCount << ", " << graph->size() << ")!" << endl;
		if (edgeCount != (graph->edgeCount() / 2) ) cout << "***ERROR*** - Edge count is different than the sum of nodes in the biconnected components (" << edgeCount << ", " << graph->edgeCount() << ")!" << endl;

		NodeDistances nds(graph, graph->getNode(0));
		
		t1 = clock();
		distance_dict_t distances = nds.getDistances();
		timeElapsed = (clock() - t1) / static_cast<double>(CLOCKS_PER_SEC);
		cout << "Calculating distances took: " << timeElapsed << endl;
		cout << "0-->123: " << distances[123] << endl;
		cout << "# of vertices: " << graph->size() << endl;
		cout << "# of distances: " << distances.size() << endl << endl;

		t1 = clock();
		SpanningTree st(graph, graph->getNode(0));
		timeElapsed = (clock() - t1) / static_cast<double>(CLOCKS_PER_SEC);
		cout << "Calculating spanning tree took: " << timeElapsed << endl;
		graph_ptr_t tree = st.getTree();
		cout << "Tree node count: " << tree->size() << endl;
		cout << "Tree edge count: " << (tree->edgeCount() / 2) << endl;
	}
	catch (const std::exception& ex)
	{
		cout << "Exception caught: " << ex.what() << endl;
	}

	return 0;
}
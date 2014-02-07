#include <DeltaHyperbolicityToolkit\Graph.h>
#include <DeltaHyperbolicityToolkit\GraphAlgorithms.h>
#include <DeltaHyperbolicityToolkit\defs.h>
#include <DeltaHyperbolicityToolkit\FurthestNode.h>
#include <DeltaHyperbolicityToolkit\NodeDistances.h>
#include <DeltaHyperbolicityToolkit\SpanningTree.h>
#include <iostream>
#include <time.h>

using namespace std;
using namespace dhtoolkit;

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
		GraphAlgorithms::pruneTrees(graph);
		timeElapsed = (clock() - t1) / static_cast<double>(CLOCKS_PER_SEC);
		cout << "Pruning took " << timeElapsed << " seconds" << endl;
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
			/*
			cout << "Sub-Graph #" << ++i << " - " << (*it)->getTitle().c_str() << ":" << endl;
			cout << "\t# of vertices: " << (*it)->size() << endl;
			cout << "\t# of edges: " << ((*it)->edgeCount() / 2) << endl << endl;
			*/
			nodeCount += (*it)->size();
			edgeCount += ((*it)->edgeCount() / 2);
		}

		if (nodeCount < graph->size()) cout << "***ERROR*** - Node count is smaller than the sum of nodes in the biconnected components (" << nodeCount << ", " << graph->size() << ")!" << endl;
		if (edgeCount != (graph->edgeCount() / 2) ) cout << "***ERROR*** - Edge count is different than the sum of nodes in the biconnected components (" << edgeCount << ", " << graph->edgeCount() << ")!" << endl;

		/*
		t1 = clock();
		GraphAlgorithms::DoubleSweepResult res = GraphAlgorithms::DoubleSweep(graph, graph->getNode(0));
		timeElapsed = (clock() - t1) / static_cast<double>(CLOCKS_PER_SEC);

		cout << "first: " << res.u->getIndex() << endl;
		cout << "second: " << res.v->getIndex() << endl;
		cout << "distance: " << res.dist << endl;
		cout << "time elapsed: " << timeElapsed << endl << endl;

		FurthestNode fns(graph, graph->getNode(0));
		node_ptr_collection_t furthest = fns.getFurthestNodes();
		distance_t distance = fns.getFurthestDistance();
		cout << "Distance: " << distance << endl;
		cout << "Found " << furthest.size() << " such nodes" << endl;
		for (node_ptr_collection_t::const_iterator it = furthest.cbegin(); it != furthest.cend(); ++it)
		{
			cout << "Node " << (*it)->getIndex() << endl;
		}
		*/
		NodeDistances nds(graph, graph->getNode(0));
		/*
		t1 = clock();
		distance = nds.getDistance(graph->getNode(300));
		timeElapsed = (clock() - t1) / static_cast<double>(CLOCKS_PER_SEC);
		cout << "0-->300: " << distance << endl;
		cout << "time elapsed: " << timeElapsed << endl << endl;

		node_ptr_collection_t nodes;
		nodes.push_back(graph->getNode(100));
		nodes.push_back(graph->getNode(200));
		nodes.push_back(graph->getNode(graph->size()-1));
		t1 = clock();
		distance_dict_t distances = nds.getDistances(nodes);
		timeElapsed = (clock() - t1) / static_cast<double>(CLOCKS_PER_SEC);
		cout << "0-->100: " << distances[100] << endl;
		cout << "0-->200: " << distances[200] << endl;
		cout << "0-->" << (graph->size()-1) << ": " << distances[graph->size()-1] << endl;
		cout << "time elapsed: " << timeElapsed << endl << endl;
		*/
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
		GraphAlgorithms::pruneTrees(tree);
		cout << "After pruning, original has: " << st.getTree()->size() << " nodes" << endl;
		cout << "After pruning: " << tree->size() << ", " << (tree->edgeCount() / 2) << endl << endl;
	}
	catch (const std::exception& ex)
	{
		cout << "Exception caught: " << ex.what() << endl;
	}

	return 0;
}
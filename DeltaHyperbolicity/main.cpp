#include <iostream>
#include <Windows.h>
#include <string>
#include <time.h>
#include <sstream>
#include "GraphAlgorithms.h"
#include "defs.h"
#include "IGraphAlg.h"

using namespace std;
using namespace graphs;

typedef IGraphAlg* (*AlgCreationMethod)();
typedef void (*AlgReleaseMethod)(IGraphAlg*);

graph_ptr_t graph;

/*
 * @returns	The algorithm loaded from the given dll.
 * @throws	std::exception	Upon any error (missing dll / expected methods).
 */
shared_ptr<IGraphAlg> loadAlgorithm(LPCSTR algDllPath)
{
	cout << "Loading " << algDllPath << "..." << endl;

	shared_ptr<HINSTANCE__> alg(LoadLibrary(algDllPath), &CloseHandle);
	if (nullptr == alg.get()) throw std::exception("Failed loading dll");

	AlgCreationMethod createAlg = reinterpret_cast<AlgCreationMethod>(GetProcAddress(alg.get(), "CreateAlgorithm"));
	if (nullptr == createAlg) throw std::exception("Failed to get the algorithm creation method");

	AlgReleaseMethod releaseAlg = reinterpret_cast<AlgReleaseMethod>(GetProcAddress(alg.get(), "ReleaseAlgorithm"));
	if (nullptr == releaseAlg) throw std::exception("Failed to get the algorithm release method");

	return shared_ptr<IGraphAlg>(createAlg(), releaseAlg);
}

void printMenu()
{
	cout << "****************************" << endl;
	cout << "* 1. Load new graph.       *" << endl;
	cout << "* 2. Run an algorithm.     *" << endl;
	cout << "* 3. Exit.                 *" << endl;
	cout << "****************************" << endl;
	cout << endl;
}

void loadNewGraph()
{
	string graphPath;
	cout << "Enter the new graph path: ";
	getline(cin, graphPath);

	try
	{
		graph = GraphAlgorithms::LoadGraphFromFile(graphPath.c_str());
		
		//display stats before & after pruning trees
		cout << "Graph loaded, " << graph->size() << " nodes and " << graph->edgeCount() << " edges have been loaded!" << endl;
		cout << "Pruning..." << endl;
		GraphAlgorithms::PruneTrees(graph);
		cout << "Graph now has " << graph->size() << " nodes and " << graph->edgeCount() << " edges." << endl;
	}
	catch (const std::exception& e)
	{
		cout << "An error occurred loading the graph: " << e.what() << endl;
	}
}

void runAlgorithm()
{
	//make sure a graph has already been loaded
	if (nullptr == graph.get())
	{
		cout << "You must load a graph first!" << endl;
		return;
	}

	string input;
	cout << "Enter algorithm dll name/path: ";
	getline(cin, input);

	try
	{
		//load & run the algorithm
		shared_ptr<IGraphAlg> alg = loadAlgorithm(input.c_str());
		clock_t t1 = clock();
		DeltaHyperbolicity delta = alg->run(graph);
		double timeElapsed = (clock() - t1) / static_cast<double>(CLOCKS_PER_SEC);
		cout << "Run with no initial state:" << endl;
		cout << "Delta: " << delta.getDelta() << endl;
		cout << "Nodes: ";
		for (unsigned int i = 0; i < NodeQuadCount-1; ++i)
		{
			cout << delta.getNodes()[i]->getIndex() << ", ";
		}
		cout << delta.getNodes()[NodeQuadCount-1]->getIndex() << endl;
		cout << "Run time: " << timeElapsed << endl;
	}
	catch (const std::exception& ex)
	{
		cout << "Algorithm threw an exception: " << ex.what() << endl;
	}
}

int main()
{
	try
	{
		string input;
		unsigned int choice = 0;

		printMenu();
		cout << "Please select from the menu above: ";
		getline(cin, input);
		stringstream(input) >> choice;

		//start loop, loading algorithms requested by the user (unless "exit" is typed)
		while (3 != choice)
		{
			cout << endl;
			switch (choice)
			{
			case 1:
				loadNewGraph();
				break;

			case 2:
				runAlgorithm();
				break;
			}
			cout << endl;

			//get next choice
			printMenu();
			cout << "Please select from the menu above: ";
			getline(cin, input);
			stringstream(input) >> choice;
		}
	}
	catch (const std::exception& e)
	{
		cout << "Exception caught: " << e.what() << endl;
	}

	return 0;
}
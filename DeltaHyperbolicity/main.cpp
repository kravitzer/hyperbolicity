#include "AlgRunner.h"
#include <iostream>
#include <Windows.h>
#include <string>
#include <time.h>
#include <sstream>
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\IGraphAlg.h"

using namespace std;
using namespace graphs;

graph_ptr_t graph;


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

	//note: alg runner must be declared *outside* of the try/catch block, because an exception thrown
	//inside the block may be thrown from the dll itself, in which case having it inside the block would
	//cause it to be destructed (i.e. the dll freed) before the exception instance is destroyed! When trying
	//to deallocate the exception instance, an access violation will occur as the dll is no longer loaded.
	AlgRunner alg(input);
	try
	{
		//load & run the algorithm
		alg.load();
		clock_t t1 = clock();
		DeltaHyperbolicity delta = alg.run(graph);
		double timeElapsed = (clock() - t1) / static_cast<double>(CLOCKS_PER_SEC);
		cout << "Run with no initial state:" << endl;
		cout << "Delta: " << delta.getDelta() << endl;
		cout << "Nodes: ";
		for (unsigned int i = 0; i < node_quad_t::size()-1; ++i)
		{
			cout << delta.getNodes()[i]->getIndex() << ", ";
		}
		cout << delta.getNodes()[node_quad_t::size()-1]->getIndex() << endl;
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
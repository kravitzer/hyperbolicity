#include <iostream>
#include <Windows.h>
#include <string>
#include <time.h>
#include "GraphAlgorithms.h"
#include "defs.h"
#include "IGraphAlg.h"

using namespace std;
using namespace graphs;

typedef IGraphAlg* (*AlgCreationMethod)();
typedef void (*AlgReleaseMethod)(IGraphAlg*);

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


int main()
{
	try
	{
		//load the graph
		graph_ptr_t g = GraphAlgorithms::LoadGraphFromFile("C:\\Users\\Eran\\Dropbox\\University\\Thesis\\Shavitt\\Code\\Graphs\\Generated\\100_300\\2.txt");
	
		//display stats before & after pruning trees
		cout << "Graph loaded, " << g->size() << " nodes and " << g->edgeCount() << " edges have been loaded!" << endl;
		cout << "Pruning..." << endl;
		GraphAlgorithms::PruneTrees(g);
		cout << "Graph now has " << g->size() << " nodes and " << g->edgeCount() << " edges." << endl;
		cout << endl;

		string input;
		cout << "Enter algorithm dll path: ";
		getline(cin, input);

		//start loop, loading algorithms requested by the user (unless "exit" is typed)
		while (0 != _stricmp(input.c_str(), "exit"))
		{
			try
			{
				//load & run the algorithm
				shared_ptr<IGraphAlg> alg = loadAlgorithm(input.c_str());
				cout << endl;
				clock_t t1 = clock();
				DeltaHyperbolicity delta = alg->run(g);
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

			//get next algorithm to run
			cout << endl;
			cout << "Enter algorithm dll path: ";
			getline(cin, input);
		}
	}
	catch (const std::exception& e)
	{
		cout << "Exception caught: " << e.what() << endl;
	}

	return 0;
}
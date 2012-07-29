#include "AlgRunner.h"
#include <iostream>
#include <Windows.h>
#include <string>
#include <time.h>
#include <sstream>
#include <vector>
#include <memory>
#include <iomanip>
#include <stdio.h>
#include <boost\filesystem.hpp>
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\IGraphAlg.h"

using namespace std;
using namespace dhtoolkit;
namespace fs = boost::filesystem;

typedef vector<graph_ptr_t>			graph_collection_t;
typedef shared_ptr<FILE>			file_ptr_t;
typedef shared_ptr<AlgRunner>		alg_runner_ptr_t;
typedef vector<alg_runner_ptr_t>	alg_runner_collection_t;

graph_collection_t graphs;
string graphsTitle;


//delete functor for shared pointer to an array (AKA shared array)
template <typename T>
struct arrayDeleter
{
	void operator()(T* p)
	{
		delete [] p;
	}
};

void printMenu()
{
	cout << "****************************" << endl;
	cout << "* 1. Load new graph.       *" << endl;
	cout << "* 2. Load graph directory. *" << endl;
	cout << "* 3. Run an algorithm(s).  *" << endl;
	cout << "* 4. Exit.                 *" << endl;
	cout << "****************************" << endl;
	cout << endl;
}

void loadGraph(string graphPath)
{
	graphs.push_back( GraphAlgorithms::LoadGraphFromFile(graphPath.c_str()) );
	
	//if graph path is too long, trim the beginning, and display it
	const unsigned int MaxPathLen = 20;
	if (graphPath.length() > MaxPathLen) graphPath = "..." + graphPath.substr(graphPath.length()-MaxPathLen);
	cout << "Graph " << graphPath.c_str() << " loaded successfully!" << endl;

	//display stats before & after pruning trees
	cout << "Graph has " << graphs[0]->size() << " nodes and " << graphs[0]->edgeCount() << " edges." << endl;
	cout << "Pruning..." << endl;
	GraphAlgorithms::PruneTrees(graphs[0]);
	cout << "Graph now has " << graphs[0]->size() << " nodes and " << graphs[0]->edgeCount() << " edges." << endl;
}

void loadSingleGraph()
{
	string graphPath;
	cout << "Enter the new graph path: ";
	getline(cin, graphPath);
	cout << endl;

	try
	{
		graphs.clear();
		loadGraph(graphPath);
	}
	catch (const exception& ex)
	{
		cout << "An error occurred loading the graph: " << ex.what() << endl;
	}
}

void loadGraphDirectory()
{
	string dirPath;
	cout << "Enter the directory path: ";
	getline(cin, dirPath);
	cout << endl;

	try
	{
		if (!fs::is_directory(dirPath))
		{
			cout << "Directory enterred does not exist" << endl;
			return;
		}

		graphs.clear();
		fs::recursive_directory_iterator end;
		for (fs::recursive_directory_iterator it(dirPath); it != end; ++it)
		{
			try
			{
				fs::path curPath = it->path();
				if (fs::is_regular(curPath))
				{
					loadGraph(curPath.string());
					cout << endl;
				}
			}
			catch (const exception&)
			{
				//skip errors
			}
		}

		if (0 == graphs.size())
		{
			cout << "No valid graph files have been found, no graphs have been loaded!" << endl;
		}
		else
		{
			cout << graphs.size() << " graphs have been loaded!" << endl;
		}
	}
	catch (const std::exception& e)
	{
		cout << "An error occurred while iterating the directory: " << e.what() << endl;
	}
}

void writeStringToFile(file_ptr_t f, const string& str)
{
	if (str.length() != fwrite(str.c_str(), sizeof(char), str.length(), f.get()) )
	{
		throw exception("Failed writing string to file");
	}
}

file_ptr_t createRawDataFile(const string& filePath, const alg_runner_collection_t& algorithms)
{
	//create output file
	FILE* outputFile = _fsopen(filePath.c_str(), "w", _SH_DENYWR);
	if (nullptr == outputFile)
	{
		throw exception("Failed opening file");
	}
	file_ptr_t f(outputFile, &fclose);

	//write file header
	writeStringToFile(f, "file, ");

	//for each algorithm X, write: "X Delta, X Run Time, " ...
	for (alg_runner_collection_t::const_iterator it = algorithms.cbegin(); it != algorithms.cend(); ++it)
	{
		string algName = (*it)->getName();
		writeStringToFile(f, algName);
		writeStringToFile(f, " Delta, ");
		writeStringToFile(f, algName);
		writeStringToFile(f, " Run Time, ");
	}

	//move back 2 places to overwrite the last comma with a newline
	fseek(f.get(), -2, SEEK_CUR);
	writeStringToFile(f, "\n");

	return f;
}

file_ptr_t createSummaryFile(const string& filePath, const alg_runner_collection_t& algorithms)
{
	//create output file
	FILE* outputFile = _fsopen(filePath.c_str(), "w", _SH_DENYWR);
	if (nullptr == outputFile)
	{
		throw exception("Failed opening file");
	}
	file_ptr_t f(outputFile, &fclose);

	//write file header
	writeStringToFile(f, "file, ");

	//for each algorithm X, write: "X Best Delta, X Avg Delta, X Worst Delta, X Delta Distribution, X Best Time, X Avg Time, X Worst Time, X Time Variance"
	for (alg_runner_collection_t::const_iterator it = algorithms.cbegin(); it != algorithms.cend(); ++it)
	{
		string algName = (*it)->getName();
		writeStringToFile(f, algName);
		writeStringToFile(f, " Best Delta, ");
		writeStringToFile(f, algName);
		writeStringToFile(f, " Avg Delta, ");
		writeStringToFile(f, algName);
		writeStringToFile(f, " Worst Delta, ");
		writeStringToFile(f, algName);
		writeStringToFile(f, " Delta Distribution, ");
		writeStringToFile(f, algName);
		writeStringToFile(f, " Best Time, ");
		writeStringToFile(f, algName);
		writeStringToFile(f, " Avg Time, ");
		writeStringToFile(f, algName);
		writeStringToFile(f, " Worst Time, ");
		writeStringToFile(f, algName);
		writeStringToFile(f, " Time Variance, ");
	}

	//move back 2 places to overwrite the last comma with a newline
	fseek(f.get(), -2, SEEK_CUR);
	writeStringToFile(f, "\n");

	return f;
}

string getCurrentTime()
{
	time_t now;
	time(&now);
	tm curTime = {0};
	localtime_s(&curTime, &now);

	stringstream s;
	s << setfill('0');
	s << (1900+curTime.tm_year) << setw(2) << (1+curTime.tm_mon) << setw(2) << curTime.tm_mday << "_" << setw(2) << curTime.tm_hour << setw(2) << curTime.tm_min << setw(2) << curTime.tm_sec;
	return s.str();
}

void runAlgorithms(unsigned int runsPerGraph)
{
	//make sure a graph has already been loaded
	if (0 == graphs.size())
	{
		cout << "You must load a graph first!" << endl;
		return;
	}

	stringstream fileName;
	fileName << getCurrentTime() << "_" << graphsTitle << "_";

	alg_runner_collection_t algorithms;
	string input;
	cout << "Enter algorithm dll name/path: ";
	getline(cin, input);
	while (0 != input.length())
	{
		//note: alg runner must be declared *outside* of the try/catch block, because an exception thrown
		//inside the block may be thrown from the dll itself, in which case having it inside the block would
		//cause it to be destructed (i.e. the dll freed) before the exception instance is destroyed! When trying
		//to deallocate the exception instance, an access violation will occur as the dll is no longer loaded.
		alg_runner_ptr_t alg(new AlgRunner(input));
		try
		{
			alg->load();
			algorithms.push_back(alg);
			fileName << alg->getName() << "_";
		}
		catch (const exception& ex)
		{
			cout << "An error occurred while loading the algorithm: " << ex.what() << endl;
		}

		cout << "Enter algorithm dll name/path (leave empty if done): ";
		getline(cin, input);
	}

	if (0 == algorithms.size())
	{
		cout << "You must load at least one algorithm!" << endl;
		return;
	}

	string fileNameStr = fileName.str();
	file_ptr_t rawFile = createRawDataFile(fileNameStr.substr(0, fileNameStr.length()-1) + "_raw.csv", algorithms);
	file_ptr_t sumFile = createSummaryFile(fileNameStr.substr(0, fileNameStr.length()-1) + "_sum.csv", algorithms);
	
	for (graph_collection_t::const_iterator graphIt = graphs.cbegin(); graphIt != graphs.cend(); ++graphIt)
	{
		graph_ptr_t graph = *graphIt;
		writeStringToFile(sumFile, graph->getTitle() + ", ");

		const double InfiniteTime = -1;
		shared_ptr<delta_t> maxDelta(new delta_t[algorithms.size()]);
		shared_ptr<delta_t> minDelta(new delta_t[algorithms.size()]);
		shared_ptr<delta_t> deltaSum(new delta_t[algorithms.size()]);
		shared_ptr<unordered_map<delta_t, unsigned int>> deltaDistributions(new unordered_map<delta_t, unsigned int>[algorithms.size()], arrayDeleter<unordered_map<delta_t, unsigned int>>());
		shared_ptr<double> maxTime(new delta_t[algorithms.size()]);
		shared_ptr<double> minTime(new delta_t[algorithms.size()]);
		shared_ptr<double> timeSum(new delta_t[algorithms.size()]);
		shared_ptr<double> timeSquareSum(new delta_t[algorithms.size()]);
		for (unsigned int i = 0; i < algorithms.size(); ++i)
		{
			maxDelta.get()[i] = 0;
			minDelta.get()[i] = InfiniteDelta;
			deltaSum.get()[i] = 0;
			maxTime.get()[i] = 0;
			minTime.get()[i] = InfiniteTime;
			timeSum.get()[i] = 0;
			timeSquareSum.get()[i] = 0;
		}

		for (unsigned int i = 0; i < runsPerGraph; ++i)
		{
			bool isFirstRun = true;
			State curState;
			unsigned int algIndex = 0;
			writeStringToFile(rawFile, graph->getTitle() + ", ");
			for (alg_runner_collection_t::const_iterator algIt = algorithms.cbegin(); algIt != algorithms.cend(); ++algIt, ++algIndex)
			{
				try
				{
					//load & run the algorithm
					DeltaHyperbolicity delta(0);
					clock_t t1 = clock();
					if (isFirstRun)
					{
						delta = (*algIt)->run(graph);
						isFirstRun = false;
					}
					else
					{
						delta = (*algIt)->runWithInitialState(graph, curState);
					}
					double timeElapsed = (clock() - t1) / static_cast<double>(CLOCKS_PER_SEC);
					curState = delta.getNodes();

					//update summarized info
					if (maxDelta.get()[algIndex] < delta.getDelta()) maxDelta.get()[algIndex] = delta.getDelta();
					if ( (minDelta.get()[algIndex] == InfiniteDelta) || (minDelta.get()[algIndex] > delta.getDelta()) ) minDelta.get()[algIndex] = delta.getDelta();
					deltaSum.get()[algIndex] += delta.getDelta();
					if (deltaDistributions.get()[algIndex].find(delta.getDelta()) != deltaDistributions.get()[algIndex].end())
					{
						++deltaDistributions.get()[algIndex][delta.getDelta()];
					}
					else
					{
						deltaDistributions.get()[algIndex][delta.getDelta()] = 1;
					}

					if (maxTime.get()[algIndex] < timeElapsed) maxTime.get()[algIndex] = timeElapsed;
					if ( (minTime.get()[algIndex] == InfiniteTime) || (minTime.get()[algIndex] > timeElapsed) ) minTime.get()[algIndex] = timeElapsed;
					timeSum.get()[algIndex] += timeElapsed;
					timeSquareSum.get()[algIndex] += timeElapsed * timeElapsed;

					//print run results to screen
					cout << "Delta: " << delta.getDelta() << endl;
					cout << "Nodes: ";
					for (unsigned int i = 0; i < node_quad_t::size()-1; ++i)
					{
						cout << delta.getNodes()[i]->getIndex() << ", ";
					}
					cout << delta.getNodes()[node_quad_t::size()-1]->getIndex() << endl;
					cout << "Run time: " << timeElapsed << endl << endl;

					//write results to raw data file
					stringstream rawData;
					rawData << delta.getDelta() << ", " << timeElapsed;
					if (algIndex != algorithms.size()-1) rawData << ", ";
					writeStringToFile(rawFile, rawData.str());
					fflush(rawFile.get());
				}
				catch (const std::exception& ex)
				{
					cout << "Algorithm threw an exception: " << ex.what() << endl;
				}
			}

			writeStringToFile(rawFile, "\n");
		}

		stringstream sumData;
		for (unsigned int i = 0; i < algorithms.size(); ++i)
		{
			sumData << maxDelta.get()[i] << ", " << deltaSum.get()[i] / static_cast<double>(runsPerGraph) << ", " << minDelta.get()[i] << ", ";
			for (unordered_map<delta_t, unsigned int>::const_iterator it = deltaDistributions.get()[i].cbegin(); it != deltaDistributions.get()[i].cend(); ++it)
			{
				sumData << "{" << it->first << ":" << it->second << "} ";
			}
			double timeAvg = timeSum.get()[i] / static_cast<double>(runsPerGraph);
			double timeVar = (timeSquareSum.get()[i] - timeAvg*timeSum.get()[i]) / static_cast<double>(runsPerGraph);
			sumData << ", " << minTime.get()[i] << ", " << timeAvg << ", " << maxTime.get()[i] << ", " << timeVar << ", ";
		}

		string sumDataStr = sumData.str();
		writeStringToFile(sumFile, sumDataStr.substr(0, sumDataStr.length()-2) + "\n");

		fflush(sumFile.get());
	}
}

int main()
{
	const unsigned int RunsPerGraph = 5;
	try
	{
		string input;
		unsigned int choice = 0;

		printMenu();
		cout << "Please select from the menu above: ";
		getline(cin, input);
		stringstream(input) >> choice;

		//start loop, loading algorithms requested by the user (unless "exit" is typed)
		while (4 != choice)
		{
			cout << endl;
			switch (choice)
			{
			case 1:
				loadSingleGraph();
				break;

			case 2:
				loadGraphDirectory();
				break;

			case 3:
				runAlgorithms(RunsPerGraph);
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
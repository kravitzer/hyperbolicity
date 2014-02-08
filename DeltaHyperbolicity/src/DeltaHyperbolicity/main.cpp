#include <iostream>
#include <Windows.h>
#include <string>
#include <time.h>
#include <sstream>
#include <vector>
#include <memory>
#include <iomanip>
#include <stdio.h>
#include <random>
#include <boost\filesystem.hpp>
#include <boost\tokenizer.hpp>
#include <boost\lexical_cast.hpp>
#include <boost\format.hpp>
#include "Graph\AlgRunner.h"
#include "Graph\GraphAlgorithms.h"
#include "Graph\defs.h"
#include "Graph\IGraphAlg.h"
#include "Graph\NodeDistances.h"
#include "Graph\FurthestNode.h"
#include "Graph\SpanningTree.h"

using namespace std;
using namespace dhtoolkit;
namespace fs = boost::filesystem;

typedef vector<graph_ptr_t>			graph_collection_t;
typedef shared_ptr<FILE>			file_ptr_t;
typedef shared_ptr<AlgRunner>		alg_runner_ptr_t;
typedef vector<alg_runner_ptr_t>	alg_runner_collection_t;
typedef boost::tokenizer< boost::char_separator<char> > tokenizer;

struct GraphBreakdown
{
	GraphBreakdown(string graphTitle, const graph_collection_t& graphBreakdown) : title(graphTitle), graphs(graphBreakdown) {}

	string	title;
	graph_ptr_collection_t graphs;
};

vector<GraphBreakdown> graphs;
string outputDir = ".\\";
bool shouldCalculateUpperBound = true;
bool shouldProduceRawFiles = true;

const unsigned int SecondsBetweenWriteToRaw = 30;

//delete functor for shared pointer to an array (AKA shared array)
template <typename T>
struct arrayDeleter
{
	void operator()(T* p)
	{
		delete [] p;
	}
};

string getComputerName()
{
	const uint32_t InitialBufLen = 100;

	DWORD bufLen = InitialBufLen;
	vector<char> usernameBuffer(bufLen);


	if (0 == GetComputerNameA(&usernameBuffer[0], &bufLen))
	{
		DWORD err = GetLastError();
		//error code can represent an insufficient buffer size, or some other error. We can handle insufficient buffer size,
		//if it's any other error - throw exception.
		if (ERROR_INSUFFICIENT_BUFFER != err)
		{
			throw exception("Failed to retrieve computer name");
		}

		//ok, our buffer size is not large enough, let's take care of that. bufLen now holds the necessary length (including the null-terminator).
		usernameBuffer.resize(bufLen);
		if (0 == GetComputerNameA(&usernameBuffer[0], &bufLen))
		{
			//still an error?? this time we know the buffer size is sufficient, so we're gonna have to declare failure
			throw exception("Failed to retrieve computer name");
		}
	}

	//if we've reached this point - we've got the user name! The buffer is guaranteed to be null-terminated (by the API) so we
	//can just let the string class parse it up to the null character, and return the result.
	return string(&usernameBuffer[0]);
}

void printMenu()
{
	cout << "***************************************" << endl;
	cout << "* 1.  Load new graph.                 *" << endl;
	cout << "* 2.  Load graph directory.           *" << endl;
	cout << "* 3.  Select output directory.        *" << endl;
	cout << "* 4.  Run an algorithm(s).            *" << endl;
	cout << "* 5.  Deprecated.                     *" << endl; //Calculate delta on given nodes. *" << endl;
	cout << "* 6.  Deprecated.                     *" << endl; //Calculate distance b/w 2 nodes. *" << endl;
	cout << "* 7.  Calculate upper bound is: " << (shouldCalculateUpperBound ? "ON " : "OFF") << "   *" << endl;
	cout << "* 8.  Produce raw result file: " << (shouldProduceRawFiles ? "ON " : "OFF") << "    *" << endl;
	cout << "* 9.  Split to bicomponents.          *" << endl;
	cout << "* 10. Convert to Pajek.               *" << endl;
	cout << "* 11. Exit.                           *" << endl;
	cout << "***************************************" << endl;
	cout << endl;
}

void loadGraph(string graphPath, unsigned int type)
{
	graph_ptr_t curGraph;
	if (1 == type)	curGraph = GraphAlgorithms::LoadGraphFromFile(graphPath.c_str());
	else if (2 == type) curGraph = GraphAlgorithms::LoadGraphFromEdgeListFile(graphPath.c_str(), true);
	else throw runtime_error("Invalid graph type entered");

	//if graph path is too long, trim the beginning, and display it
	const unsigned int MaxPathLen = 20;
	if (graphPath.length() > MaxPathLen) graphPath = "..." + graphPath.substr(graphPath.length()-MaxPathLen);
	cout << "Graph " << graphPath.c_str() << " loaded successfully!" << endl;

	//display stats before & after pruning trees
	cout << "Graph has " << curGraph->size() << " nodes and " << (curGraph->edgeCount() / 2) << " edges." << endl;

	//calculate biconnected components
	cout << "Calculating biconnected components..." << endl;
	graph_ptr_collection_t bicon = GraphAlgorithms::getBiconnectedComponents(curGraph);
	//find valid biconnected components and add each of them as a separate graph
	size_t nodeCount = 0;
	size_t edgeCount = 0;
	graph_ptr_collection_t subgraphs;
	for (graph_ptr_collection_t::const_iterator it = bicon.cbegin(); it != bicon.cend(); ++it)
	{
		if ((*it)->size() >= State::size())
		{
			subgraphs.push_back(*it);
			nodeCount += (*it)->size();
			edgeCount += (*it)->edgeCount();
		}
	}

	graphs.push_back(GraphBreakdown(curGraph->getTitle(), subgraphs));

	cout << subgraphs.size() << " biconnected components larger than " << State::size() << " nodes have been added (out of " << bicon.size() << " found)" << endl;
	cout << "Total number of nodes is " << nodeCount << " and total number of edges is " << edgeCount/2 << endl;
}

void loadSingleGraph()
{
	string graphPath;
	cout << "Enter the new graph path: ";
	getline(cin, graphPath);
	cout << endl;

	string graphType;
	cout << "Enter graph type: " << endl;
	cout << "1. Simple Type." << endl;
	cout << "2. Edge List Type." << endl;
	getline(cin, graphType);
	cout << endl;

	try
	{
		graphs.clear();
		loadGraph(graphPath, boost::lexical_cast<unsigned int>(graphType));
	}
	catch (const exception& ex)
	{
		cout << "An error occurred loading the graph: " << ex.what() << endl;
	}
}

void loadGraphDirectory(string dirPath)
{
	try
	{
		//make sure we were given a path to an existing folder
		if (!fs::is_directory(dirPath))
		{
			cout << "Directory enterred does not exist" << endl;
			return;
		}

		//clear the graph collection and iterate over all files in the folder
		graphs.clear();
		fs::recursive_directory_iterator end;
		for (fs::recursive_directory_iterator it(dirPath); it != end; ++it)
		{
			try
			{
				//if the current path is a file - try to load it
				fs::path curPath = it->path();
				if (fs::is_regular(curPath))
				{
					loadGraph(curPath.string(), 1);
					cout << endl;
				}
			}
			catch (const exception&)
			{
				//loading failed - maybe it is not a graph file? skip to next file.
			}
		}

		if (0 == graphs.size())
		{
			//not a single graph has been loaded :(
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

void loadGraphDirectory()
{
	string dirPath;
	cout << "Enter the directory path: ";
	getline(cin, dirPath);
	cout << endl;

	loadGraphDirectory(dirPath);
}

void getOutputDirectory()
{
	string newOutputDir;
	cout << "Enter a new output path (relative or absolute): ";
	getline(cin, newOutputDir);
	cout << endl;

	if (0 == newOutputDir.size()) return;
	if (newOutputDir[newOutputDir.size()-1] != '\\') newOutputDir += '\\';

	if (boost::filesystem::is_directory(newOutputDir))
	{
		outputDir = newOutputDir;
	}
	else
	{
		cout << "The entered directory does not exist!" << endl;
		cout << "Output directory remained: " << outputDir.c_str() << endl;
	}
}

void writeStringToFile(file_ptr_t f, const string& str)
{
	if (str.length() != fwrite(str.c_str(), sizeof(char), str.length(), f.get()) )
	{
		throw exception("Failed writing string to file");
	}
}

file_ptr_t createRawDataFile(const string& filePath)
{
	//create output file
	FILE* fileHandle = _fsopen(filePath.c_str(), "w", _SH_DENYWR);
	if (nullptr == fileHandle)
	{
		throw runtime_error((boost::format("Failed opening raw file: %1%") % filePath).str().c_str());
	}	
	file_ptr_t f(fileHandle, &fclose);

	//write file header
	writeStringToFile(f, "file, Delta, Run Time, Nodes\n");

	return f;
}

file_ptr_t createSummaryFile(const string& filePath)
{
	//create output file
	FILE* fileHandle = _fsopen(filePath.c_str(), "w", _SH_DENYWR);
	if (nullptr == fileHandle)
	{
		throw runtime_error((boost::format("Failed opening summary file: %1%") % filePath).str().c_str());
	}
	file_ptr_t f(fileHandle, &fclose);

	//write file headers
	writeStringToFile(f, "file, Number of Iterations, Iterations to Max Delta, Best Delta, Avg Delta, Worst Delta, Delta Distribution, Sample State, Best Time, Avg Time, Worst Time, Time Variance, Is Brute Force?, Upper Bound\n");

	return f;
}

file_ptr_t createGeneralSummaryFile(const string& filePath)
{
	//create output file
	file_ptr_t f(_fsopen(filePath.c_str(), "w", _SH_DENYWR), &fclose);
	if (nullptr == f.get())
	{
		throw exception("Failed opening file");
	}


	//write file header
	writeStringToFile(f, "file, Best Delta, Total Run Time, Is Brute Force?");
	if (shouldCalculateUpperBound) writeStringToFile(f, ", Upper Bound");
	writeStringToFile(f, "\n");

	return f;
}

string getCurrentTime()
{
	time_t now = 0;
	time(&now);
	tm curTime = {0};
	localtime_s(&curTime, &now);

	stringstream s;
	s << setfill('0');
	s << (1900+curTime.tm_year) << setw(2) << (1+curTime.tm_mon) << setw(2) << curTime.tm_mday << "_" << setw(2) << curTime.tm_hour << setw(2) << curTime.tm_min << setw(2) << curTime.tm_sec;
	return s.str();
}

delta_t calculateUpperBoundWithSpanningTree(const graph_ptr_t g)
{
	//search for nodes with highest degree
	node_ptr_collection_t highestDegreeNodes;
	highestDegreeNodes.push_back(g->getNode(0));
	size_t highestDegree = g->getNode(0)->getEdges().size();
	for (unsigned int i = 1; i < g->size(); ++i)
	{
		node_ptr_t cur = g->getNode(i);
		if (cur->getEdges().size() >= highestDegree)
		{
			if (cur->getEdges().size() > highestDegree)
			{
				highestDegree = cur->getEdges().size();
				highestDegreeNodes.clear();
			}

			highestDegreeNodes.push_back(cur);
		}
	}

	const unsigned int MaxNodesToTry = 5;
	delta_t lowestBound = InfiniteDelta;
	//for each of the nodes with highest degree, up to the maximal number of trials,
	//calculate spanning tree & calculate bound based on it. Keep the best bound found.
	for (unsigned int i = 0; i < MaxNodesToTry && i < highestDegreeNodes.size(); ++i)
	{
		//calculate spanning tree & estimate diameter based on it.
		const graph_ptr_t tree = SpanningTree(g, highestDegreeNodes[i]).getTree();
		node_ptr_t furthest1 = FurthestNode(tree, tree->getNode(0)).getFurthestNodes()[0];
		distance_t diameter = FurthestNode(tree, furthest1).getFurthestDistance();

		//d1 <= diameter*2, delta <= d1/4 <= diameter*2/4
		delta_t currentBound = static_cast<delta_t>(diameter)*2/4;
		//notice that with this calculation, the bound is always either an integer or half an integer, as it should be
		//so no truncation is necessary

		//update bound if a better one was found
		if ( (InfiniteDelta == lowestBound) || (currentBound < lowestBound) ) lowestBound = currentBound;
	}

	return lowestBound;
}

delta_t calculateUpperBoundWithTrivialBound(const graph_ptr_t g)
{
	//calculate distances from an arbitrary node (i.e. the first one)
	distance_dict_t distances = NodeDistances(g, g->getNode(0)).getDistances();
	distance_t largestDistances[4];

	//take the first 4 distances to be the maximal ones for now
	distance_dict_t::const_iterator it = distances.cbegin();
	largestDistances[0] = it->second;
	++it;
	largestDistances[1] = it->second;
	++it;
	largestDistances[2] = it->second;
	++it;
	largestDistances[3] = it->second;

	//sort those 4 distances such that the largest is first, lowest is last
	for (int i = 0; i < 4; ++i)
	{
		for (int j = i+1; j < 4; ++j)
		{
			if (largestDistances[j-1] < largestDistances[j])
			{
				int temp = largestDistances[j];
				largestDistances[j] = largestDistances[j-1];
				largestDistances[j-1] = temp;
			}
		}
	}

	//go thru all other distances in the distance map. Keep only the 4 largest distances after each step
	for (; it != distances.cend(); ++it)
	{
		int k = 4;
		if (it->second > largestDistances[0]) k = 0;
		else if (it->second > largestDistances[1]) k = 1;
		else if (it->second > largestDistances[2]) k = 2;
		else if (it->second > largestDistances[3]) k = 3;

		for (int i = k; i < 3; ++i)
		{
			largestDistances[i+1] = largestDistances[i];
		}
		if (k < 4) largestDistances[k] = it->second;
	}

	//upper bound for d1 is the sum of the 4 top distances
	distance_t d1bound = largestDistances[0] + largestDistances[1] + largestDistances[2] + largestDistances[3];
	delta_t deltaBound = d1bound / 4.0;
	//Note that we may get results like X.25 or X.75 which are impossible values for delta (where X is 
	//some integer). So we check first, and truncate if necessary to the nearest half-integer
	if ( (d1bound % 4 == 1) || (d1bound % 4 == 3) ) deltaBound -= 0.25;

	return deltaBound;
}

delta_t calculateUpperBound(graph_ptr_t g)
{
	delta_t b1 = calculateUpperBoundWithSpanningTree(g);
	delta_t b2 = calculateUpperBoundWithTrivialBound(g);

	return (b1 < b2 ? b1 : b2);
}

void runGivenAlgorithms(alg_runner_collection_t algorithms, const vector<file_ptr_t>& rawFiles, const vector<file_ptr_t>& sumFiles, file_ptr_t generalSumFile, unsigned int runsPerGraph, unsigned int timeLimit)
{
	const unsigned int BruteForceThreshold = 100;
	alg_runner_collection_t* algsToRun = &algorithms;
	
	//try to load BruteForce algorithm to use on graph below size threshold
	bool bfAvailable = true;
	alg_runner_collection_t bfAlg(1);
	try
	{
		bfAlg[0] = alg_runner_ptr_t(new AlgRunner("BruteForce"));
	}
	catch (const std::exception& ex)
	{
		//failed to load algorithm - print warning and set flag signaling that we cannot use it
		cout << "Note - failed to load BruteForce algorithm: " << ex.what() << endl;
		bfAvailable = false;
	}

	for (vector<GraphBreakdown>::const_iterator graphIt = graphs.cbegin(); graphIt != graphs.cend(); ++graphIt)
	{
		stringstream generalSumData;
		generalSumData << graphIt->title << ", ";

		try
		{
			//get next graph
			graph_ptr_collection_t graph = graphIt->graphs;

			delta_t bestDelta = 0;
			clock_t initialTime = clock();
			DeltaHyperbolicity bestDH;
			delta_t maxUpperBound = 0;
			bool allRunsWithBf = true;

			for (graph_collection_t::const_iterator subgraphIt = graph.cbegin(); subgraphIt != graph.cend(); ++subgraphIt)
			{
				graph_ptr_t curGraph(new Graph(**subgraphIt));
				//write next graph to all sum files
				for (vector<file_ptr_t>::const_iterator it = sumFiles.cbegin(); it != sumFiles.cend(); ++it)
				{
					writeStringToFile(*it, curGraph->getTitle() + ", ");
				}

				unsigned int algIndex = 0;
				algsToRun = &algorithms;
				bool runBf = curGraph->size() < BruteForceThreshold;
				if (bfAvailable && runBf)
				{
					algsToRun = &bfAlg;
				}
				else
				{
					allRunsWithBf = false;
				}
				for (alg_runner_collection_t::const_iterator algIt = algsToRun->cbegin(); algIt != algsToRun->cend(); ++algIt, ++algIndex)
				{
					//initialize statistical variables
					const double InfiniteTime = -1;
					node_combination_t maxDeltaState;
					delta_t maxDelta = 0;
					delta_t minDelta = InfiniteDelta;
					delta_t deltaSum = 0;
					unordered_map<delta_t, unsigned int> deltaDistributions;
					double maxTime = 0;
					double minTime = InfiniteTime;
					long double timeSum = 0;
					double timeSquareSum = 0;
					uint64_t numOfIterationsToMax = 0;
					double lastRawWriteTime = (-static_cast<int>(SecondsBetweenWriteToRaw)-1) * CLOCKS_PER_SEC;
					uint64_t runCount = 0;

					//calculate upper bound for subgraph
					delta_t upperBound = 0;
					if (shouldCalculateUpperBound)
					{
						upperBound = calculateUpperBound(curGraph);
						if (upperBound > maxUpperBound) maxUpperBound = upperBound;
					}

					try
					{
						clock_t initialAlgTime = clock();
						(*algIt)->initialize(curGraph, bestDH.getState());
						clock_t initFinishTime = clock();
						cout << "Algorithm finished initialization process in " << initFinishTime - initialAlgTime << " milliseconds" << endl;

						for (runCount = 0; !(*algIt)->isComplete() && ( (clock()-initialAlgTime) / static_cast<double>(CLOCKS_PER_SEC) < timeLimit || 0 == timeLimit) && (runCount < runsPerGraph || 0 == runsPerGraph); ++runCount)
						{
							//run another step, get best delta found so far
							clock_t t1 = clock();
							DeltaHyperbolicity delta = (*algIt)->step();
							double timeElapsed = (clock() - t1) / static_cast<double>(CLOCKS_PER_SEC);

							//if the best delta is still uninitialized - initialize it now with the current state
							if (!bestDH.getState().isInitialized())
							{
								bestDH.set(maxDelta, delta.getState());
							}

							//if max delta for current algorithm was improved - update it
							if ( maxDelta < delta.getDelta() || !maxDeltaState.isInitialized() ) 
							{
								maxDelta = delta.getDelta();
								maxDeltaState = delta.getState();
								numOfIterationsToMax = runCount+1;

								//check if best delta was improved by this new finding
								if (bestDH.getDelta() < maxDelta) 
								{
									bestDH.set(maxDelta, delta.getState());
								}
							}

							//update summarized info
							if ( (minDelta == InfiniteDelta) || (minDelta > delta.getDelta()) ) minDelta = delta.getDelta();
							deltaSum += delta.getDelta();
							if (deltaDistributions.find(delta.getDelta()) != deltaDistributions.end())
							{
								++deltaDistributions[delta.getDelta()];
							}
							else
							{
								deltaDistributions[delta.getDelta()] = 1;
							}

							if (maxTime < timeElapsed) maxTime = timeElapsed;
							if ( (minTime == InfiniteTime) || (minTime > timeElapsed) ) minTime = timeElapsed;
							timeSum += timeElapsed;
							timeSquareSum += timeElapsed * timeElapsed;

							//write results to raw data file as long as at least 30 seconds have passed since last write time, or if this is the last write for this graph
							stringstream rawData;
							rawData << curGraph->getTitle() << ", " << delta.getDelta() << ", " << timeElapsed << ", {" << delta.printNodes() << "}\n";
							if (shouldProduceRawFiles) 
							{
								writeStringToFile(rawFiles[algIndex], rawData.str());
								double now = clock();
								if ( ( (now - lastRawWriteTime) / static_cast<double>(CLOCKS_PER_SEC) > SecondsBetweenWriteToRaw) || 
									!(runCount+1 < runsPerGraph || (0 == runsPerGraph && !(*algIt)->isComplete() ) ) )
								{
									fflush(rawFiles[algIndex].get());

									lastRawWriteTime = now;
								}
							}
						}

						//write summary to summary-file
						stringstream sumData;
						sumData << runCount << ", ";
						if (0 == runCount)
						{
							sumData << "n/a, n/a, n/a, n/a, {}, n/a, n/a, n/a, n/a, n/a, n/a, " << upperBound << "\n";
						}
						else
						{
							sumData << numOfIterationsToMax << ", " << maxDelta << ", " << deltaSum / static_cast<double>(runCount) << ", " << minDelta << ", ";
							for (unordered_map<delta_t, unsigned int>::const_iterator it = deltaDistributions.cbegin(); it != deltaDistributions.cend(); ++it)
							{
								sumData << "{" << it->first << ":" << it->second << "} ";
							}
							if (!maxDeltaState.isInitialized()) throw std::exception("Max delta state should be initialized by this point");
							sumData << ", {" << maxDeltaState.printNodes() << "}, ";
							double timeAvg = timeSum / static_cast<long double>(runCount);
							double timeVar = (timeSquareSum - timeAvg*timeSum) / static_cast<double>(runCount);
							sumData << minTime << ", " << timeAvg << ", " << maxTime << ", " << timeVar << ", " << runBf << ", " << upperBound << "\n";
						}

						writeStringToFile(sumFiles[algIndex], sumData.str());
						fflush(sumFiles[algIndex].get());
					}
					catch (const std::exception& ex)
					{
						cout << "Algorithm threw an exception: " << ex.what() << endl;
						stringstream exceptionMessage;
						exceptionMessage << ", " << "Exception: " << ex.what() << "\n";
						if (shouldProduceRawFiles)
						{
							stringstream rawMessage;
							rawMessage << curGraph->getTitle() << exceptionMessage.str();
							writeStringToFile(rawFiles[algIndex], rawMessage.str());
							fflush(sumFiles[algIndex].get());
						}

						writeStringToFile(sumFiles[algIndex], exceptionMessage.str());
						fflush(sumFiles[algIndex].get());

						throw;
					}
					catch (...)
					{
						cout << "Algorithm threw an unknown exception!" << endl;
						stringstream exceptionMessage;
						exceptionMessage << ", " << "Unknown Exception!\n";
						if (shouldProduceRawFiles)
						{
							stringstream rawMessage;
							rawMessage << curGraph->getTitle() << exceptionMessage.str();
							writeStringToFile(rawFiles[algIndex], rawMessage.str());
							fflush(sumFiles[algIndex].get());
						}

						writeStringToFile(sumFiles[algIndex], exceptionMessage.str());
						fflush(sumFiles[algIndex].get());

						throw;
					}
				}
			}

			double timeElapsed = (clock() - initialTime) / static_cast<double>(CLOCKS_PER_SEC);

			generalSumData << bestDH.getDelta() << ", " << timeElapsed << ", " << allRunsWithBf <<  ", ";

			if (shouldCalculateUpperBound)
			{
				generalSumData << maxUpperBound << ", ";
			}

			string generalSumStr = generalSumData.str();
			writeStringToFile(generalSumFile, generalSumStr.substr(0, generalSumStr.size()-2) + '\n');

			fflush(generalSumFile.get());
		}
		catch (const std::exception& ex)
		{
			generalSumData << "Exception: " << ex.what() << endl;
			writeStringToFile(generalSumFile, generalSumData.str());
			fflush(generalSumFile.get());
		}
		catch (...)
		{
			generalSumData << "Unknown Exception!" << endl;
			writeStringToFile(generalSumFile, generalSumData.str());
			fflush(generalSumFile.get());
		}
	}
}

void runAlgorithms()
{
	//make sure a graph has already been loaded
	if (0 == graphs.size())
	{
		cout << "You must load a graph first!" << endl;
		return;
	}

	//get # of runs per graph
	string runsPerGraphStr;
	cout << "Please enter the number of times to run on each graph (0 means no limit): ";
	getline(cin, runsPerGraphStr);
	int runsPerGraph = atoi(runsPerGraphStr.c_str());

	//get time limit
	unsigned int timeLimit = 0;
	cout << "Please enter a time limit in seconds (0 means no limit): ";
	getline(cin, runsPerGraphStr);
	timeLimit = atoi(runsPerGraphStr.c_str());

	if (0 == runsPerGraph && 0 == timeLimit) cout << "NOTE: Some algorithms do not end voluntarily!" << endl;

	//file collections
	vector<file_ptr_t> rawFiles;
	vector<file_ptr_t> sumFiles;

	//file names are "<current-date&time>_<graph-title>_<comp-name>_<alg_1>_<alg_2>...<alg_n>.csv"
	//so far we only have the date/time, graph title & computer name...
	stringstream fileName;
	fileName << outputDir << getCurrentTime() << "_" << getComputerName() << "_";

	//receive input for algorithm names
	alg_runner_collection_t algorithms;
	string input;
	cout << "Enter algorithm shared library name/path: ";
	getline(cin, input);
	while (0 != input.length())
	{
		//note: alg runner must be declared *outside* of the try/catch block, because an exception thrown
		//inside the block may be thrown from the shared library itself, in which case having it inside the block would
		//cause it to be destructed (i.e. the shared library freed) before the exception instance is destroyed! When trying
		//to deallocate the exception instance, an access violation will occur as the shared library is no longer loaded.
		alg_runner_ptr_t alg;
		try
		{
			//load the algorithm shared library and add it to the algorithm collection
			alg.reset(new AlgRunner(input));
			algorithms.push_back(alg);
		}
		catch (const exception& ex)
		{
			//loading/adding of shared library failed
			cout << "An error occurred while loading the algorithm: " << ex.what() << endl;
		}

		//add algorithm name to the file name string
		fileName << alg->getName() << "_" << GetCurrentProcessId() << "_";

		//create raw & sum files for current algorithm chain
		string fileNameStr = fileName.str();
		if (shouldProduceRawFiles) rawFiles.push_back(createRawDataFile(fileNameStr.substr(0, fileNameStr.length()-1) + "_raw.csv"));
		sumFiles.push_back(createSummaryFile(fileNameStr.substr(0, fileNameStr.length()-1) + "_sum.csv"));

		//get next algorithm input
		cout << "Enter algorithm shared library name/path (leave empty if done): ";
		getline(cin, input);
	}

	//make sure at least one algorithm has been loaded
	if (0 == algorithms.size())
	{
		cout << "You must load at least one algorithm!" << endl;
		return;
	}

	string fileNameStr = fileName.str();
	file_ptr_t generalSumFile = createGeneralSummaryFile(fileNameStr.substr(0, fileNameStr.length()-1) + "_general.csv");

	runGivenAlgorithms(algorithms, rawFiles, sumFiles, generalSumFile, runsPerGraph, timeLimit);
}
/*
void calculateDelta()
{
if (1 != graphs.size())
{
cout << "You must have one and only one graph loaded to use this feature!" << endl;
return;
}

graph_ptr_t g = graphs[0];

string input;
cout << "Enter the node indices, separated by commas: ";
getline(cin, input);

boost::char_separator<char> sep(", ");
tokenizer tokens(input, sep);

tokenizer::iterator it = tokens.begin();
node_ptr_t nodes[4];
for (int i = 0; i < 4; ++i)
{
nodes[i] = g->getNode( boost::lexical_cast<int>(*it) );
++it;
}

node_combination_t state(nodes[0], nodes[1], nodes[2], nodes[3]);
cout << "The delta value for this state is: " << GraphAlgorithms::CalculateDelta(g, state) << endl;
}

void calculateDistance()
{
if (1 != graphs.size())
{
cout << "You must have one and only one graph loaded to use this feature!" << endl;
return;
}

graph_ptr_t g = graphs[0];

string input;
cout << "Enter the node indices, separated by commas: ";
getline(cin, input);

boost::char_separator<char> sep(", ");
tokenizer tokens(input, sep);

//create collection of nodes to find distances between
node_ptr_collection_t nodes;
for (tokenizer::iterator it = tokens.begin(); it != tokens.end(); ++it)
{
nodes.push_back( g->getNode( boost::lexical_cast<int>(*it) ) );
}

//find distances
for (node_ptr_collection_t::const_iterator it = nodes.cbegin(); it != nodes.cend(); ++it)
{
//current node will be the origin
node_ptr_t origin = *it;
//create destination collection with the nodes that come after the origin in the collection
node_ptr_collection_t dests;
for (node_ptr_collection_t::const_iterator it2 = it+1; it2 != nodes.cend(); ++it2)
{
dests.push_back(*it2);
}
//find the distances & print them!
NodeDistances nds(g, origin);
distance_dict_t distances = nds.getDistances(dests);
for (distance_dict_t::const_iterator distanceIt = distances.cbegin(); distanceIt != distances.cend(); ++distanceIt)
{
cout << origin->getIndex() << "\tto\t" << distanceIt->first << ":\t" << distanceIt->second << endl;
}
}
}
*/
void printUsage(char* imageName)
{
	cout << "Usage (ui):\n\t" << imageName << endl;
	cout << "Usage (single execution):\n\t" << imageName << " -i input-file -o output-dir -n num-of-executions -t time-limit [-R] -a algorithm1 algorithm2 ..." << endl;
	cout << "-R: do NOT produce raw files" << endl;
}

void commandLineExecution(int argc, char** argv)
{
	if (argc < 11)
	{
		printUsage(argv[0]);
		return;
	}

	if (_stricmp(argv[1], "-i") != 0 || _stricmp(argv[3], "-o") != 0 || _stricmp(argv[5], "-n") != 0 || _stricmp(argv[7], "-t") != 0 || 
		( _stricmp(argv[9], "-a") != 0 && (_stricmp(argv[9], "-R") != 0 || _stricmp(argv[10], "-a") != 0) ) )
	{
		printUsage(argv[0]);
		return;
	}

	char* input = argv[2];
	string myOutputDir(argv[4]);
	unsigned int n = atoi(argv[6]);
	unsigned int timeLimit = atoi(argv[8]);

	int algCmdIndex = 10;
	if (_stricmp(argv[9], "-R") == 0)
	{
		shouldProduceRawFiles = false;
		++algCmdIndex;
	}
	else
	{
		shouldProduceRawFiles = true;
	}

	vector<string> algs;
	for (; algCmdIndex < argc; ++algCmdIndex)
	{
		algs.push_back(string(argv[algCmdIndex]));
	}


	//load inputs
	if (boost::filesystem::is_directory(input))
	{
		loadGraphDirectory(input);
	}
	else
	{
		loadGraph(input, 1);
	}

	//set output folder
	if (myOutputDir[myOutputDir.size()-1] != '\\') myOutputDir += '\\';

	if (boost::filesystem::is_directory(myOutputDir))
	{
		outputDir = myOutputDir;
	}
	else
	{
		throw std::exception("Invalid output directory");
	}

	//file collections
	vector<file_ptr_t> rawFiles;
	vector<file_ptr_t> sumFiles;

	//run algorithm!
	alg_runner_collection_t algorithms;
	stringstream fileName;
	fileName << outputDir << getCurrentTime() << "_" << getComputerName() << "_";
	for (vector<string>::const_iterator it = algs.cbegin(); it != algs.cend(); ++it)
	{
		//note: alg runner must be declared *outside* of the try/catch block, because an exception thrown
		//inside the block may be thrown from the shared library itself, in which case having it inside the block would
		//cause it to be destructed (i.e. the shared library freed) before the exception instance is destroyed! When trying
		//to deallocate the exception instance, an access violation will occur as the shared library is no longer loaded.
		alg_runner_ptr_t alg;
		try
		{
			//load the algorithm shared library and add it to the algorithm collection
			alg.reset(new AlgRunner(*it));
			algorithms.push_back(alg);
		}
		catch (const exception& ex)
		{
			//loading/adding of shared library failed
			cout << "An error occurred while loading the algorithm: " << ex.what() << endl;
		}

		fileName << alg->getName() << "_" << GetCurrentProcessId() << "_";

		string fileNameStr = fileName.str();
		if (shouldProduceRawFiles) rawFiles.push_back(createRawDataFile(fileNameStr.substr(0, fileNameStr.length()-1) + "_raw.csv"));
		sumFiles.push_back(createSummaryFile(fileNameStr.substr(0, fileNameStr.length()-1) + "_sum.csv"));
	}

	string fileNameStr = fileName.str();
	file_ptr_t generalSumFile = createGeneralSummaryFile(fileNameStr.substr(0, fileNameStr.length()-1) + "_general.csv");

	runGivenAlgorithms(algorithms, rawFiles, sumFiles, generalSumFile, n, timeLimit);
}

void uiExecution()
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
		while (11 != choice)
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
				getOutputDirectory();
				break;

			case 4:
				runAlgorithms();
				break;

			case 5:
				//calculateDelta();
				break;

			case 6:
				//calculateDistance();
				break;

			case 7:
				shouldCalculateUpperBound = !shouldCalculateUpperBound;
				break;

			case 8:
				shouldProduceRawFiles = !shouldProduceRawFiles;
				break;

			case 9:
				if (0 == graphs.size() || "" == outputDir)
				{
					cout << "Invalid input graph / output dir" << endl;
					break;
				}

				for (vector<GraphBreakdown>::const_iterator it = graphs.cbegin(); it != graphs.cend(); ++it)
				{
					for (graph_collection_t::const_iterator graphIt = it->graphs.cbegin(); graphIt != it->graphs.cend(); ++graphIt)
					{
						string title = (*graphIt)->getTitle();
						for (size_t nextIndex = title.find('\\'); nextIndex != string::npos; nextIndex = title.find('\\'))
						{
							title.replace(nextIndex, 1, "@");
						}

						string path = outputDir + title;
						cout << "Saving graph to path: " << path.c_str() << endl;
						GraphAlgorithms::SaveGraphToFile(*graphIt, path);
					}
				}
				break;

			case 10:
				if (0 == graphs.size() || "" == outputDir)
				{
					cout << "Invalid input graph / output dir" << endl;
					break;
				}

				for (vector<GraphBreakdown>::const_iterator it = graphs.cbegin(); it != graphs.cend(); ++it)
				{
					for (graph_collection_t::const_iterator graphIt = it->graphs.cbegin(); graphIt != it->graphs.cend(); ++graphIt)
					{
						string title = (*graphIt)->getTitle();
						for (size_t nextIndex = title.find('\\'); nextIndex != string::npos; nextIndex = title.find('\\'))
						{
							title.replace(nextIndex, 1, "@");
						}
						title += ".pajek";

						string path = outputDir + title;
						cout << "Saving graph to path: " << path.c_str() << endl;
						GraphAlgorithms::drawGraph(path, *graphIt, nullptr);
					}
				}
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
	catch (const boost::exception&)
	{
		cout << "Boost exception caught" << endl;
	}
	catch (const std::exception& e)
	{
		cout << "Exception caught: " << e.what() << endl;
	}
}

int main(int argc, char** argv)
{
	if (1 == argc)
	{
		uiExecution();
	}
	else
	{
		commandLineExecution(argc, argv);
	}

	return 0;
}
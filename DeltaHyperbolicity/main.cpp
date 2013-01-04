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
#include <boost\tokenizer.hpp>
#include <boost\lexical_cast.hpp>
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\IGraphAlg.h"
#include "DeltaHyperbolicityToolkit\NodeDistances.h"
#include "DeltaHyperbolicityToolkit\FurthestNode.h"
#include "DeltaHyperbolicityToolkit\SpanningTree.h"

using namespace std;
using namespace dhtoolkit;
namespace fs = boost::filesystem;

typedef vector<graph_ptr_t>			graph_collection_t;
typedef shared_ptr<FILE>			file_ptr_t;
typedef shared_ptr<AlgRunner>		alg_runner_ptr_t;
typedef vector<alg_runner_ptr_t>	alg_runner_collection_t;
typedef boost::tokenizer< boost::char_separator<char> > tokenizer;

graph_collection_t graphs;
string graphsTitle;
string outputDir = ".\\";
bool shouldCalculateUpperBound = true;

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
	cout << "**************************************" << endl;
	cout << "* 1. Load new graph.                 *" << endl;
	cout << "* 2. Load graph directory.           *" << endl;
	cout << "* 3. Select output directory.        *" << endl;
	cout << "* 4. Run an algorithm(s).            *" << endl;
	cout << "* 5. Calculate delta on given nodes. *" << endl;
    cout << "* 6. Calculate upper bound is: " << (shouldCalculateUpperBound ? "ON " : "OFF") << "   *" << endl;
	cout << "* 7. Exit.                           *" << endl;
	cout << "**************************************" << endl;
	cout << endl;
}

void loadGraph(string graphPath)
{
	graph_ptr_t curGraph = GraphAlgorithms::LoadGraphFromFile(graphPath.c_str());
	graphs.push_back(curGraph);
	
	//if graph path is too long, trim the beginning, and display it
	const unsigned int MaxPathLen = 20;
	if (graphPath.length() > MaxPathLen) graphPath = "..." + graphPath.substr(graphPath.length()-MaxPathLen);
	cout << "Graph " << graphPath.c_str() << " loaded successfully!" << endl;

	//display stats before & after pruning trees
	cout << "Graph has " << curGraph->size() << " nodes and " << curGraph->edgeCount() << " edges." << endl;
	cout << "Pruning..." << endl;
	curGraph->pruneTrees();
	cout << "Graph now has " << curGraph->size() << " nodes and " << curGraph->edgeCount() << " edges." << endl;
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
                    loadGraph(curPath.string());
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

file_ptr_t createRawDataFile(const string& filePath, const alg_runner_collection_t& algorithms)
{
	//create output file
	file_ptr_t f(_fsopen(filePath.c_str(), "w", _SH_DENYWR), &fclose);
	if (nullptr == f.get())
	{
		throw exception("Failed opening file");
	}	

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
		writeStringToFile(f, algName);
		writeStringToFile(f, " Nodes, ");
	}

	//move back 2 places to overwrite the last comma with a newline
	fseek(f.get(), -2, SEEK_CUR);
	writeStringToFile(f, "\n");

	return f;
}

file_ptr_t createSummaryFile(const string& filePath, const alg_runner_collection_t& algorithms)
{
	//create output file
	file_ptr_t f(_fsopen(filePath.c_str(), "w", _SH_DENYWR), &fclose);
	if (nullptr == f.get())
	{
		throw exception("Failed opening file");
	}
	

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

    if (shouldCalculateUpperBound) writeStringToFile(f, "Upper Bound, ");

	//move back 2 places to overwrite the last comma with a newline
	fseek(f.get(), -2, SEEK_CUR);
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
	node_collection_t highestDegreeNodes;
	highestDegreeNodes.push_back(g->getNode(0));
	unsigned int highestDegree = g->getNode(0)->getEdges().size();
	
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

    //upper bound is the sum of the 4 top distances over 4 (if not an integer, will be rounded down as it should)
    return static_cast<delta_t>(largestDistances[0] + largestDistances[1] + largestDistances[2] + largestDistances[3]) / 4;
}

delta_t calculateUpperBound(graph_ptr_t g)
{
	delta_t b1 = calculateUpperBoundWithSpanningTree(g);
	delta_t b2 = calculateUpperBoundWithTrivialBound(g);

	return (b1 < b2 ? b1 : b2);
}

void runGivenAlgorithms(alg_runner_collection_t algorithms, file_ptr_t rawFile, file_ptr_t sumFile, unsigned int runsPerGraph)
{
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
					rawData << delta.getDelta() << ", " << timeElapsed << ", {" << delta.printNodes() << "}";
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

        if (shouldCalculateUpperBound)
        {
            delta_t upperBound = calculateUpperBound(graph);
            sumData << upperBound << ", ";
        }

		string sumDataStr = sumData.str();
		writeStringToFile(sumFile, sumDataStr.substr(0, sumDataStr.length()-2) + "\n");

		fflush(sumFile.get());
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
	cout << "Please enter the number of times to run on each graph: ";
	getline(cin, runsPerGraphStr);
	int runsPerGraph = atoi(runsPerGraphStr.c_str());

	//file names are "<current-date&time>_<graph-title>_<comp-name>_<alg_1>_<alg_2>...<alg_n>.csv"
	//so far we only have the date/time, graph title & computer name...
	stringstream fileName;
	fileName << outputDir << getCurrentTime() << "_" << graphsTitle << "_" << getComputerName() << "_";

	//receive input for algorithm names
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
		alg_runner_ptr_t alg(new AlgRunner(input, outputDir));
		try
		{
			//load the algorithm dll and add it to the algorithm collection
			alg->load();
			algorithms.push_back(alg);
		}
		catch (const exception& ex)
		{
			//loading/adding of dll failed
			cout << "An error occurred while loading the algorithm: " << ex.what() << endl;
		}

		//add algorithm name to the file name string
		fileName << alg->getName() << "_";

		//get next algorithm input
		cout << "Enter algorithm dll name/path (leave empty if done): ";
		getline(cin, input);
	}

	//make sure at least one algorithm has been loaded
	if (0 == algorithms.size())
	{
		cout << "You must load at least one algorithm!" << endl;
		return;
	}

	//add ".csv" extension to the file name
	string fileNameStr = fileName.str();
	file_ptr_t rawFile = createRawDataFile(fileNameStr.substr(0, fileNameStr.length()-1) + "_raw.csv", algorithms);
	file_ptr_t sumFile = createSummaryFile(fileNameStr.substr(0, fileNameStr.length()-1) + "_sum.csv", algorithms);
	

	runGivenAlgorithms(algorithms, rawFile, sumFile, runsPerGraph);
}

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

	node_quad_t state(nodes[0], nodes[1], nodes[2], nodes[3]);
	cout << "The delta value for this state is: " << GraphAlgorithms::CalculateDelta(g, state) << endl;
}

void printUsage(char* imageName)
{
    cout << "Usage (ui):\t\t" << imageName << endl;
    cout << "Usage (single execution):\t" << imageName << " [-i input-file -o output-dir -n num-of-executions -a algorithm1 algorithm2 ...]" << endl;
}

void commandLineExecution(int argc, char** argv)
{
    if (argc < 9)
    {
        printUsage(argv[0]);
        return;
    }

    if (stricmp(argv[1], "-i") != 0 || stricmp(argv[3], "-o") != 0 || stricmp(argv[5], "-n") != 0 || stricmp(argv[7], "-a") != 0 )
    {
        printUsage(argv[0]);
        return;
    }

    char* input = argv[2];
    string myOutputDir(argv[4]);
    unsigned int n = atoi(argv[6]);
    vector<string> algs;
    for (int i = 8; i < argc; ++i)
    {
        algs.push_back(string(argv[i]));
    }


    //load inputs
    if (boost::filesystem::is_directory(input))
    {
        loadGraphDirectory(input);
    }
    else
    {
        loadGraph(input);
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

    //run algorithm!
    alg_runner_collection_t algorithms;
    stringstream fileName;
    fileName << outputDir << getCurrentTime() << "_" << graphsTitle << "_" << getComputerName() << "_";
    for (vector<string>::const_iterator it = algs.cbegin(); it != algs.cend(); ++it)
    {
        //note: alg runner must be declared *outside* of the try/catch block, because an exception thrown
        //inside the block may be thrown from the dll itself, in which case having it inside the block would
        //cause it to be destructed (i.e. the dll freed) before the exception instance is destroyed! When trying
        //to deallocate the exception instance, an access violation will occur as the dll is no longer loaded.
        alg_runner_ptr_t alg(new AlgRunner(*it, outputDir));
        try
        {
            //load the algorithm dll and add it to the algorithm collection
            alg->load();
            algorithms.push_back(alg);
        }
        catch (const exception& ex)
        {
            //loading/adding of dll failed
            cout << "An error occurred while loading the algorithm: " << ex.what() << endl;
        }

        fileName << alg->getName() << "_";
    }

    string fileNameStr = fileName.str();
    file_ptr_t rawFile = createRawDataFile(fileNameStr.substr(0, fileNameStr.length()-1) + "_raw.csv", algorithms);
    file_ptr_t sumFile = createSummaryFile(fileNameStr.substr(0, fileNameStr.length()-1) + "_sum.csv", algorithms);


    runGivenAlgorithms(algorithms, rawFile, sumFile, n);
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
        while (7 != choice)
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
				calculateDelta();
				break;

            case 6:
                shouldCalculateUpperBound = !shouldCalculateUpperBound;
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
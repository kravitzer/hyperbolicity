#include "GraphAlgorithms.h"
#include "defs.h"
#include "FurthestNode.h"
#include "NodeDistances.h"
#include "boost/format.hpp"
#include <memory>
#include <vector>
#include <algorithm>
#include <string>
#include <queue>
#include <unordered_set>
#include <fstream>

using namespace std;

namespace dhtoolkit
{
	const int GraphAlgorithms::NodeIndexMaxNumOfDigits = 20;		//including null-terminator
	const char* GraphAlgorithms::EdgeMarker = "->";
	const size_t GraphAlgorithms::EdgeMarkerLen = strlen(EdgeMarker);
	//calculation is: 2*nodes (null is counted twice, therefore:) minus 1, plus length of edge marker, plus length of 
	//newline character
	const int GraphAlgorithms::EdgeMaxLen = (2*NodeIndexMaxNumOfDigits - 1) + sizeof(EdgeMarker) + 1;
	const char* GraphAlgorithms::Delimiter = "\r\n";
	const size_t GraphAlgorithms::DelimiterLen = strlen(Delimiter);
#ifdef _WIN32
	const char GraphAlgorithms::PathSeparator = '\\';
#else
	const char GraphAlgorithms::PathSeparator = '/';
#endif
	

	void GraphAlgorithms::SaveGraphToFile(const graph_ptr_t graph, const std::string& path)
	{
		if (0 == graph->size()) throw exception("Cannot save empty graph!");

		shared_ptr<FILE> outputFile = OpenFile(path.c_str(), "wb", _SH_DENYRW);
		//write all nodes but last
		for (unsigned int i = 0; i < graph->size() - 1; ++i)
		{
			WriteNodeToFile(outputFile, i, false);
		}
		//write last node
		WriteNodeToFile(outputFile, graph->size()-1, true);

		//for each node, write its edges
		for (node_index_t i = 0; i < graph->size(); ++i)
		{
			node_ptr_t node = graph->getNode(i);
			const node_weak_ptr_collection_t& edges = node->getEdges();
			for (node_weak_ptr_collection_t::const_iterator edgeIt = edges.begin(); edgeIt != edges.end(); ++edgeIt)
			{
				WriteEdgeToFile(outputFile, node->getIndex(), edgeIt->lock()->getIndex());
			}
		}
	}

	graph_ptr_t GraphAlgorithms::LoadGraphFromFile(const std::string& path)
	{
		shared_ptr<FILE> inputFile = OpenFile(path.c_str(), "rb", _SH_DENYWR);
		
		//create graph
		graph_ptr_t g(new Graph(shortPath(path)));

		//create graph nodes
		unsigned int nodeCount = ReadNodeCount(inputFile);
		for (unsigned int i = 0; i < nodeCount; ++i) g->insertNode();

		//create edges
		vector<Edge> edges = ReadEdges(inputFile);
		for (vector<Edge>::const_iterator it = edges.cbegin(); it != edges.end(); ++it)
		{
			node_ptr_t src = g->getNode(it->src);
			src->insertUnidirectionalEdgeTo(g->getNode(it->dst));
		}

		return g;
	}

	graph_ptr_t GraphAlgorithms::LoadGraphFromEdgeListFile(const std::string& path, bool isBidirectional)
	{
		//create graph
		graph_ptr_t g(new Graph(shortPath(path)));

		//start reading from file
		ifstream inputFile;
		inputFile.exceptions(ios::badbit);
		inputFile.open(path.c_str());

		unordered_map<node_index_t,node_index_t> addedNodes;
		string line;
		while (!inputFile.eof())
		{
			getline(inputFile, line);

			//skip empty lines and comments
			if (line.size() == 0 || line[0] == '#') continue;
			size_t tabIndex = line.find('\t');
			if (string::npos == tabIndex) throw runtime_error("Invalid line format: Failed to find tab character");
			//if we get a pointer to the text (line.c_str()) we'll currently have <id1>\t<id2>\0
			//we set the tab to be null, this way we can treat it as two different strings - one from the beginning up to the tab (which will become null),
			//and another starting right after the tab an up until the end
			line[tabIndex] = '\0';
			node_index_t node1Index = atoi(line.c_str());
			node_index_t node2Index = atoi(line.c_str() + tabIndex + 1);

			//get node pointers (either from graph if already added before, or create and get the new node pointers)
			node_ptr_t node1,node2;
			if (addedNodes.cend() == addedNodes.find(node1Index))
			{
				node1 = g->insertNode();
				addedNodes[node1Index] = node1->getIndex();
			}
			else
			{
				node1 = g->getNode(addedNodes[node1Index]);
			}

			if (addedNodes.cend() == addedNodes.find(node2Index))
			{
				node2 = g->insertNode();
				addedNodes[node2Index] = node2->getIndex();
			}
			else
			{
				node2 = g->getNode(addedNodes[node2Index]);
			}

			//add edge(s)
			if (isBidirectional)
			{
				node1->insertBidirectionalEdgeTo(node2);
			}
			else
			{
				node1->insertUnidirectionalEdgeTo(node2);
			}
		}

		return g;
	}

	graph_ptr_collection_t GraphAlgorithms::getBiconnectedComponents(const graph_ptr_t graph)
	{
		std::unordered_map<node_index_t, unsigned int> number, lowpt;
		unsigned int index = 0;
		graph_ptr_collection_t biconnectedGraphs;
		vector<pair<node_index_t, node_index_t>> edgeStack;
		for (unsigned int nodeIndex = 0; nodeIndex < graph->size(); ++nodeIndex)
		{
			if (number.cend() == number.find(nodeIndex)) biconnected(graph, nodeIndex, -1, number, lowpt, index, edgeStack, biconnectedGraphs);
		}

		return biconnectedGraphs;
	}

	string GraphAlgorithms::shortPath(const string& path)
	{
		//find the 3rd to last path separator
		size_t slashIndex = path.size();
		for (unsigned int i = 0; (i < 3) && (slashIndex != string::npos); ++i)
		{
			slashIndex = path.find_last_of(PathSeparator, slashIndex-1);
		}
		//check if there actually are at least 3 parts - if so, return only the last 3
		if (slashIndex != string::npos) return path.substr(slashIndex+1);
		//otherwise return the original parameter
		return path;
	}

	void GraphAlgorithms::biconnected(const graph_ptr_t graph, node_index_t v, node_index_t u, unordered_map<node_index_t, unsigned int>& number, unordered_map<node_index_t, unsigned int>& lowpt, unsigned int index, vector<pair<node_index_t, node_index_t>>& edgeStack, graph_ptr_collection_t& biconnectedGraphs)
	{
		number[v] = ++index;
		lowpt[v] = number[v];
		const node_weak_ptr_collection_t& vEdges = graph->getNode(v)->getEdges();
		for (node_weak_ptr_collection_t::const_iterator it = vEdges.cbegin(); it != vEdges.cend(); ++it)
		{
			node_index_t w = it->lock()->getIndex();
			if (number.cend() == number.find(w))
			{
				edgeStack.push_back(pair<node_index_t, node_index_t>(v, w));
				biconnected(graph, w, v, number, lowpt, index, edgeStack, biconnectedGraphs);
				lowpt[v] = min(lowpt[v], lowpt[w]);
				if (lowpt[w] >= number[v])
				{
					unordered_map<node_index_t, node_index_t> indexMap;
					string title = (boost::format("%1%_%2%") % graph->getTitle() % (biconnectedGraphs.size()+1)).str();
					graph_ptr_t newGraph(new Graph(title));
					for (pair<node_index_t, node_index_t>& curEdge = edgeStack[edgeStack.size()-1]; number[curEdge.first] >= number[w]; curEdge = edgeStack[edgeStack.size()-1])
					{
						node_ptr_t u1, u2;
						//add stack top as an edge - map its elements to new node indecis
						if (indexMap.cend() == indexMap.find(curEdge.first))
						{
							u1 = newGraph->insertNode(graph->getNode(curEdge.first)->getLabel());
							indexMap[curEdge.first] = u1->getIndex();
						}
						else
						{
							u1 = newGraph->getNode(indexMap[curEdge.first]);
						}

						if (indexMap.cend() == indexMap.find(curEdge.second))
						{
							u2 = newGraph->insertNode(graph->getNode(curEdge.second)->getLabel());
							indexMap[curEdge.second] = u2->getIndex();
						}
						else
						{
							u2 = newGraph->getNode(indexMap[curEdge.second]);
						}

						u1->insertBidirectionalEdgeTo(u2);
						edgeStack.resize(edgeStack.size()-1);
					}
					edgeStack.erase(find(edgeStack.cbegin(), edgeStack.cend(), pair<node_index_t, node_index_t>(v, w)));

					node_ptr_t u1, u2;
					//add stack top as an edge - map its elements to new node indecis
					if (indexMap.cend() == indexMap.find(v))
					{
						u1 = newGraph->insertNode(graph->getNode(v)->getLabel());
						indexMap[v] = u1->getIndex();
					}
					else
					{
						u1 = newGraph->getNode(indexMap[v]);
					}

					if (indexMap.cend() == indexMap.find(w))
					{
						u2 = newGraph->insertNode(graph->getNode(w)->getLabel());
						indexMap[w] = u2->getIndex();
					}
					else
					{
						u2 = newGraph->getNode(indexMap[w]);
					}
					u1->insertBidirectionalEdgeTo(u2);

					biconnectedGraphs.push_back(newGraph);
				}
			}
			else if (number[w] < number[v] && w != u)
			{
				edgeStack.push_back(pair<node_index_t, node_index_t>(v, w));
				lowpt[v] = min(lowpt[v], number[w]);
			}
		}
	}

	shared_ptr<FILE> GraphAlgorithms::OpenFile(const char* path, const char* mode, int share)
	{
		FILE* f = _fsopen(path, mode, share);
		if (nullptr == f)
		{
			throw exception("Failed opening file");
		}
		return shared_ptr<FILE>(f, &fclose);;
	}

	void GraphAlgorithms::WriteNodeToFile(std::shared_ptr<FILE> filePtr, node_index_t nodeIndex, bool isLast)
	{
		

		//allocate memory for index's string representation
		char nodeIndexString[NodeIndexMaxNumOfDigits];
		memset(nodeIndexString, '\0', NodeIndexMaxNumOfDigits);

		//convert and format node index to a string
		int nodeIndexLen = 0;
		if (isLast)
		{
			nodeIndexLen = sprintf_s(nodeIndexString, "%d%s", nodeIndex, Delimiter);
		}
		else
		{
			nodeIndexLen = sprintf_s(nodeIndexString, "%d, ", nodeIndex);
		}

		//check for errors and write to file
		if (-1 == nodeIndexLen) throw exception("Error converting index number to string");
		if (nodeIndexLen != fwrite(nodeIndexString, sizeof(char), nodeIndexLen, filePtr.get()))
		{
			throw exception("Failed writing node to file");
		}
	}

	void GraphAlgorithms::WriteEdgeToFile(std::shared_ptr<FILE> filePtr, node_index_t srcIndex, node_index_t dstIndex)
	{
		char edgeString[EdgeMaxLen];
		memset(edgeString, '\0', EdgeMaxLen);

		int len = sprintf_s(edgeString, "%d%s%d%s", srcIndex, EdgeMarker, dstIndex, Delimiter);
		if (-1 == len) throw exception("Error creating edge string");
		if (len != fwrite(edgeString, sizeof(char), len, filePtr.get()))
		{
			throw exception("Failed writing edge to file");
		}
	}

	unsigned int GraphAlgorithms::ReadNodeCount(std::shared_ptr<FILE> filePtr)
	{
		//the nodes are a list of indices, therefore we may count the number of commas up to the newline marker
		//and add 1
		const unsigned int BufSize = 100;
		char buf[BufSize+1];
		buf[BufSize] = '\0';
		bool delimiterReached = false;
		unsigned int count = 0;

		while (!delimiterReached)
		{
			size_t bytesRead = fread(buf, sizeof(char), BufSize, filePtr.get());
			if ((BufSize != bytesRead) && (0 == feof(filePtr.get())))
			{
				throw std::exception("Failed reading from file");
			}

			for (unsigned int i = 0; (i < bytesRead) && !delimiterReached; ++i)
			{
				if (buf[i] == ',') 
				{
					++count;
				}
				else if (StringStartsWith(buf+i, Delimiter))
				{
					delimiterReached = true;
                    long newLinePosition = static_cast<long>(i - bytesRead + DelimiterLen);
					//seek back to the position of the newline (right after it, actually)
					if (0 != fseek(filePtr.get(), newLinePosition, SEEK_CUR))
					{
						throw std::exception("Failed setting file's position indicator");
					}
				}	
			}
		}

		
		return (count+1);
	}

	vector<GraphAlgorithms::Edge> GraphAlgorithms::ReadEdges(std::shared_ptr<FILE> filePtr)
	{
		vector<Edge> edges;

		const unsigned int BufSize = 100 * 1024; /* 100 KB */
		char buf[BufSize+1];
		//last character is always null, and never overwritten, to avoid a buffer overrun
		buf[BufSize] = '\0';
		char* curPos = buf;
		size_t bufDataSize = 0;

		while (0 == feof(filePtr.get()))
		{
			size_t bytesToRead = BufSize - bufDataSize;
			//if the entire buffer is occupied, it means that we couldn't find a single edge
			//marker in the entire buffer. Assuming the buffer is large enough to hold at least
			//one edge (which obviously it is), this means the file has an invalid format.
			if (0 == bytesToRead) throw std::runtime_error("Invalid graph file format");

			//read the data to buffer (to complete to BufSize)
			size_t bytesRead = fread(buf + bufDataSize, sizeof(char), bytesToRead, filePtr.get());
			if (bytesToRead != bytesRead)
			{
				if (0 == feof(filePtr.get())) throw std::exception("Failed reading from graph file");
			}

			try
			{
				while (curPos < buf + bufDataSize + bytesRead)
				{
					size_t dataRead = 0;
					Edge edge = ReadSingleEdge(curPos, true, &dataRead);
					edges.push_back(edge);
					curPos += dataRead;
				}
			}
			catch (const exception&)
			{
				//done read
			}


			bufDataSize = buf + bufDataSize + bytesRead - curPos;
			if (bufDataSize > 0) memcpy(buf, curPos, bufDataSize);
			curPos = buf;
		}

		//get the last edge in case buffer doesn't end with a delimiter
		if (bufDataSize > 0)
		{
			Edge edge = ReadSingleEdge(curPos, false, nullptr);
			edges.push_back(edge);
		}

		return edges;
	}

	GraphAlgorithms::Edge GraphAlgorithms::ReadSingleEdge(const char* buf, bool delimiterPresent, size_t* dataReadFromBuffer)
	{
		const char* edgeMarker = strstr(buf, EdgeMarker);
		if (!edgeMarker) throw std::runtime_error("Edge marker not found");
		const char* delimiter = strstr(edgeMarker + EdgeMarkerLen, Delimiter);
		if (delimiterPresent && !delimiter) throw std::runtime_error("No delimiter found");
		
		//calculate length of each node (e.g. length of node index "123" is 3, as in 3 characters)
		size_t node1len = edgeMarker - buf;
		size_t node2len = 0;
		if (delimiter)
			node2len = delimiter - edgeMarker - EdgeMarkerLen;
		else
			node2len = (buf + strlen(buf)) - edgeMarker - EdgeMarkerLen;

		//allocate space, init to null
		unique_ptr<char[]> node1(new char[node1len+1]);
		unique_ptr<char[]> node2(new char[node2len+1]);
		memset(node1.get(), '\0', node1len+1);
		memset(node2.get(), '\0', node2len+1);

		//copy node indices to char buffers
		memcpy(node1.get(), buf, node1len);
		memcpy(node2.get(), edgeMarker + EdgeMarkerLen, node2len);

		//convert to node indices
		node_index_t node1index = atoi(node1.get());
		node_index_t node2index = atoi(node2.get());
		
		Edge edge = {};
		edge.src = node1index;
		edge.dst = node2index;

		if (dataReadFromBuffer) *dataReadFromBuffer = node1len + EdgeMarkerLen + node2len + DelimiterLen;

		return edge;
	}

	bool GraphAlgorithms::StringStartsWith(const char* s, const char *p)
	{
		unsigned int i = 0;
		for (; (s[i] != '\0' && p[i] != '\0'); ++i)
		{
			if (s[i] != p[i]) return false;
		}

		return (i == strlen(p));
	}
} // namespace dhtoolkit
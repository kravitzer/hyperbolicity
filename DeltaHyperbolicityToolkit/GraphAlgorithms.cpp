#include "GraphAlgorithms.h"
#include "Except.h"
#include "defs.h"
#include "FurthestNode.h"
#include "NodeDistances.h"
#include "boost/format.hpp"
#include <memory>
#include <vector>
#include <algorithm>
#include <string>
#include <queue>

using namespace std;

namespace dhtoolkit
{
	const int GraphAlgorithms::NodeIndexMaxNumOfDigits = 20;		//including null-terminator
	const char* GraphAlgorithms::EdgeMarker = "->";
	const unsigned int GraphAlgorithms::EdgeMarkerLen = strlen(EdgeMarker);
	//calculation is: 2*nodes (null is counted twice, therefore:) minus 1, plus length of edge marker, plus length of 
	//newline character
	const int GraphAlgorithms::EdgeMaxLen = (2*NodeIndexMaxNumOfDigits - 1) + sizeof(EdgeMarker) + 1;
	const char* GraphAlgorithms::Delimiter = "\r\n";
	const unsigned int GraphAlgorithms::DelimiterLen = strlen(Delimiter);
	

	void GraphAlgorithms::SaveGraphToFile(const graph_ptr_t graph, const std::string& path)
	{
		if (0 == graph->size()) throw exception("Cannot save empty graph!");

		shared_ptr<FILE> outputFile = OpenFile(path.c_str(), "w", _SH_DENYRW);
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
			const node_collection_t& edges = node->getEdges();
			for (node_collection_t::const_iterator edgeIt = edges.begin(); edgeIt != edges.end(); ++edgeIt)
			{
				WriteEdgeToFile(outputFile, node->getIndex(), (*edgeIt)->getIndex());
			}
		}
	}

	graph_ptr_t GraphAlgorithms::LoadGraphFromFile(const std::string& path)
	{
		shared_ptr<FILE> inputFile = OpenFile(path.c_str(), "rb", _SH_DENYWR);
		
		//create graph
		string title = path;
		size_t slashIndex = title.size();
		for (unsigned int i = 0; (i < 3) && (slashIndex != string::npos); ++i)
		{
			slashIndex = title.find_last_of('\\', slashIndex-1);
		}
		if (slashIndex != string::npos) title = title.substr(slashIndex+1);
		graph_ptr_t g(new Graph(title));

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

	void GraphAlgorithms::drawGraph(const string& fileName, const graph_ptr_t graph, const node_quad_t* nodesToMark)
	{
		shared_ptr<FILE> file(_fsopen(fileName.c_str(), "wb", _SH_DENYRW), &fclose);
		if (nullptr == file.get())
		{
			throw exception("Failed creating Pajek file");
		}

		string vertices;
		vertices += (boost::format("*Vertices %1%\n") % graph->size()).str();

		string edges = "*EdgesList\n";

		for (unsigned int i = 0; i < graph->size(); ++i)
		{
			node_ptr_t curNode = graph->getNode(i);
			node_collection_t neighbors = curNode->getEdges();

			if (isNodeToBeMarked(curNode, nodesToMark))
			{
				vertices += (boost::format("%1% \"%2%\" ic Blue\n") % (i+1) % i).str();
			}
			else
			{
				vertices += (boost::format("%1% \"%2%\" ic Red\n") % (i+1) % i).str();
			}

			edges += (boost::format("%1%") % (i+1)).str();
			for (node_collection_t::const_iterator it = neighbors.cbegin(); it != neighbors.cend(); ++it)
			{
				edges += (boost::format(" %1%") % ((**it).getIndex() + 1)).str();
			}
			edges += "\n";
		}

		if (vertices.size() != fwrite(vertices.c_str(), sizeof(char), vertices.size(), file.get()))
		{
			throw exception("Error writing graph nodes to file");
		}
		if (edges.size() != fwrite(edges.c_str(), sizeof(char), edges.size(), file.get()))
		{
			throw exception("Error writing graph edges to file");
		}
	}

	delta_t GraphAlgorithms::CalculateDelta(const graph_ptr_t graph, const node_quad_t& state)
	{
		node_collection_t nodeCollection;
		nodeCollection.push_back(state[1]);
		nodeCollection.push_back(state[2]);
		nodeCollection.push_back(state[3]);

		//calculate distances from v0 to v1, v2, v3
		NodeDistances NDFromS0(graph, state[0]);
		distance_dict_t v0dists = NDFromS0.getDistances(nodeCollection);

		//calculate distances from v1 to v2, v3
		nodeCollection.erase(nodeCollection.begin());
		NodeDistances NDFromS1(graph, state[1]);
		distance_dict_t v1dists = NDFromS1.getDistances(nodeCollection);

		//calculate distances from v2 to v3
		nodeCollection.erase(nodeCollection.begin());
		NodeDistances NDFromS2(graph, state[2]);
		distance_dict_t v2dists = NDFromS2.getDistances(nodeCollection);

		//d1 = dist(v0, v1) + dist(v2, v3)
		//d2 = dist(v0, v2) + dist(v1, v3)
		//d3 = dist(v0, v3) + dist(v1, v2)
		distance_t d1 = v0dists[state[1]->getIndex()] + v2dists[state[3]->getIndex()];
		distance_t d2 = v0dists[state[2]->getIndex()] + v1dists[state[3]->getIndex()];
		distance_t d3 = v0dists[state[3]->getIndex()] + v1dists[state[2]->getIndex()];

		return CalculateDeltaFromDistances(d1, d2, d3);
	}

	delta_t GraphAlgorithms::CalculateDeltaFromDistances(distance_t d1, distance_t d2, distance_t d3)
	{
		//find largest and second-largest distances out of the 3 options
		distance_t largest = d1, secondLargest = d1;
		if (d2 > largest)
		{
			largest = d2;
		}
		else
		{
			secondLargest = d2;
		}

		if (d3 > largest)
		{
			secondLargest = largest;
			largest = d3;
		}
		else
		{
			if (d3 > secondLargest) secondLargest = d3;
		}

		//return the delta value
		return static_cast<delta_t>(largest-secondLargest)/2;
	}

	GraphAlgorithms::DoubleSweepResult GraphAlgorithms::DoubleSweep(const graph_ptr_t graph, const node_ptr_t origin /* = node_ptr_t(nullptr) */)
	{
		node_ptr_t startNode = origin;
		if (nullptr == startNode.get())
		{
			//randomly select start node
			startNode = graph->getNode(rand() % graph->size());
		}

		//perform the double sweep
		FurthestNode fn(graph, startNode);
		node_ptr_t firstSweepNode = fn.getFurthestNodes()[0];
		distance_t dist = 0;
		distance_dict_t distanceCollection;
		node_ptr_t secondSweepNode = Sweep(graph, firstSweepNode, &dist, &distanceCollection);

		//prepare the result structure and return it to caller
		GraphAlgorithms::DoubleSweepResult res = {0};
		res.u = firstSweepNode;
		res.v = secondSweepNode;
		res.dist = dist;
		res.uDistances = distanceCollection;

		return res;
	}

	bool GraphAlgorithms::isNodeToBeMarked(node_ptr_t node, const node_quad_t* nodesToMark)
	{
		if (nullptr == nodesToMark) return false;

		for (unsigned int i = 0; i < nodesToMark->size(); ++i)
		{
			if (nodesToMark->operator[](i) == node) return true;
		}

		return false;
	}

	node_ptr_t GraphAlgorithms::Sweep(const graph_ptr_t graph, const node_ptr_t origin, distance_t* dist, distance_dict_t* distancesFromU)
	{
		NodeDistances NDFromOrigin(graph, origin);
		distance_dict_t startNodeDistances = NDFromOrigin.getDistances();
		if (distancesFromU) *distancesFromU = startNodeDistances;

		//find nodes at maximal distance
        node_collection_t furthestNodes;
		distance_t maxDistance = 0;
		for (distance_dict_t::const_iterator it = startNodeDistances.cbegin(); it != startNodeDistances.cend(); ++it)
		{
			if (InfiniteDistance == it->second) continue;
			if (it->second >= maxDistance)
            {
                if (it->second > maxDistance)
                {
                    furthestNodes.clear();
                    maxDistance = it->second;
                }
                furthestNodes.push_back(graph->getNode(it->first));
            }
		}

		//randomly select node out of the furthest nodes
		unsigned int selectedNodeIndex = rand() % furthestNodes.size();
		if (dist) *dist = maxDistance;
		return furthestNodes[selectedNodeIndex];
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

		int len = sprintf_s(edgeString, "%s%s%s%s", srcIndex, EdgeMarker, dstIndex, Delimiter);
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
			unsigned int bytesRead = fread(buf, sizeof(char), BufSize, filePtr.get());
			if ((BufSize != bytesRead) && (0 != feof(filePtr.get())))
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
					//seek back to the position of the newline (right after it, actually)
					if (0 != fseek(filePtr.get(), i - bytesRead + DelimiterLen, SEEK_CUR))
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
		unsigned int bufDataSize = 0;

		while (0 == feof(filePtr.get()))
		{
			size_t bytesToRead = BufSize - bufDataSize;
			//if the entire buffer is occupied, it means that we couldn't find a single edge
			//marker in the entire buffer. Assuming the buffer is large enough to hold at least
			//one edge (which obviously it is), this means the file has an invalid format.
			if (0 == bytesToRead) throw InvalidFormatException("Invalid graph file format");

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
					unsigned int dataRead = 0;
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

	GraphAlgorithms::Edge GraphAlgorithms::ReadSingleEdge(const char* buf, bool delimiterPresent, unsigned int* dataReadFromBuffer)
	{
		const char* edgeMarker = strstr(buf, EdgeMarker);
		if (!edgeMarker) throw InvalidFormatException("Edge marker not found");
		const char* delimiter = strstr(edgeMarker + EdgeMarkerLen, Delimiter);
		if (delimiterPresent && !delimiter) throw InvalidFormatException("No delimiter found");
		
		//calculate length of each node (e.g. length of node index "123" is 3, as in 3 characters)
		unsigned int node1len = edgeMarker - buf;
		unsigned int node2len = 0;
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
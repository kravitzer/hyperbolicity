#include "GraphAlgorithms.h"
#include "Except.h"
#include "defs.h"
#include "FurthestNode.h"
#include "NodeDistances.h"
#include "StronglyConnectedComponent.h"
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
			node_weak_ptr_collection_t neighbors = curNode->getEdges();

			if (isNodeToBeMarked(curNode, nodesToMark))
			{
				vertices += (boost::format("%1% \"%2%\" ic Blue\n") % (i+1) % i).str();
			}
			else
			{
				vertices += (boost::format("%1% \"%2%\" ic Red\n") % (i+1) % i).str();
			}

			edges += (boost::format("%1%") % (i+1)).str();
			for (node_weak_ptr_collection_t::const_iterator it = neighbors.cbegin(); it != neighbors.cend(); ++it)
			{
				edges += (boost::format(" %1%") % (it->lock()->getIndex() + 1)).str();
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
		node_ptr_collection_t nodeCollection;
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

	node_quad_t GraphAlgorithms::getRandomState(const graph_ptr_t graph)
	{
		//choose initial state at random
		node_quad_t randomState;
		for (unsigned int i = 0; i < node_quad_t::size(); ++i)
		{
			bool isUnique = true;

			//repeat the following process:
			//select a random node, then make sure none of the previous nodes is the same as this one
			do
			{
                //select a random node
				node_index_t index = rand() % graph->size();
				randomState[i] = graph->getNode(index);

                //assume unique until proven otherwise
                isUnique = true;

				//compare to all previous nodes
				for (unsigned int j = 0; (j < i) && isUnique; ++j)
				{
					if (randomState[j]->getIndex() == index) isUnique = false;
				}
			} while (!isUnique);

			//node at place i has been selected, move on to next index
		}

		return randomState;
	}

	graph_ptr_collection_t GraphAlgorithms::getStronglyConnectedComponents(const graph_ptr_t graph)
	{
		//collection of sccs
		typedef vector<node_unordered_set_ptr_t> node_unordered_set_ptr_collection_t;
		node_unordered_set_ptr_collection_t components;

		//collection to return
		graph_ptr_collection_t graphs;

		//number of nodes already assigned to an scc
		size_t processedNodeCount = 0;

		//go over all nodes in graph, an search for their strongly-connected-component
		for (unsigned int i = 0; processedNodeCount < graph->size(); ++i)
		{
			node_ptr_t curNode = graph->getNode(i);
			bool isFoundInSomeComponent = false;
			for (node_unordered_set_ptr_collection_t::const_iterator componentIt = components.cbegin(); componentIt != components.cend() && !isFoundInSomeComponent; ++componentIt)
			{
				isFoundInSomeComponent =  ( (*componentIt)->find(curNode) != (*componentIt)->cend() );
			}

			//if node is not in any of the currently known components, find its strongly-connected-component and add to collection
			if (!isFoundInSomeComponent)
			{
				//find node's scc
				StronglyConnectedComponent scc(graph, graph->getNode(i));
				node_unordered_set_ptr_t curComponent = scc.getNodes();
				string title = (boost::format("%1%_%2%") % graph->getTitle() % (graphs.size()+1)).str();
				graphs.push_back(graph_ptr_t(new Graph(title, curComponent)));
				components.push_back(curComponent);
				processedNodeCount += curComponent->size();
			}
		}

		return graphs;
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

	bool GraphAlgorithms::removeCycle(graph_ptr_t graph, node_ptr_t origin, delta_t& delta, unordered_set<string>& processedNodes)
	{
		//the chains of nodes from each side of origin to be deleted, if possible (excluding origin)
		node_ptr_collection_t chain1, chain2;

		node_weak_ptr_collection_t neighbors = origin->getEdges();
		if (neighbors.size() != 2) return 0;

		node_weak_ptr_t n1 = neighbors[0];
		node_weak_ptr_t n2 = neighbors[1];

		//the nodes that end the cycle
		node_ptr_t s, t;

		//start walking on chain1
		node_weak_ptr_t prev = origin;
		node_weak_ptr_t cur1 = neighbors[0];
		node_weak_ptr_collection_t curNeighbors = cur1.lock()->getEdges();
		for (; curNeighbors.size() == 2; curNeighbors = cur1.lock()->getEdges())
		{
			chain1.push_back(cur1.lock());

			node_weak_ptr_t tmp = cur1;
			cur1 = (curNeighbors[0].lock() == prev.lock() ? curNeighbors[1] : curNeighbors[0]);
			prev = tmp;

			//check extreme case: graph is a cycle?
			if (cur1.lock() == neighbors[1].lock())
			{
				//graph is a single cycle!
				delta_t curDelta = cycleDelta(graph->size());
				graph.reset(new Graph(graph->getTitle()));
				delta = curDelta;
				return true;
			}
		}

		//start walking on chain2
		prev = origin;
		node_weak_ptr_t cur2 = neighbors[1];
		curNeighbors = cur2.lock()->getEdges();
		for (; curNeighbors.size() == 2; curNeighbors = cur2.lock()->getEdges())
		{
			chain2.push_back(cur2.lock());

			node_weak_ptr_t tmp = cur2;
			cur2 = (curNeighbors[0].lock() == prev.lock() ? curNeighbors[1] : curNeighbors[0]);
			prev = tmp;
		}

		NodeDistances distCalculator(graph, cur1.lock());
		distance_t stDist = distCalculator.getDistance(cur2.lock());

		//add this chain's nodes to the irremovable set
		processedNodes.insert(origin->getLabel());
		for (auto it = chain1.cbegin(); it != chain1.cend(); ++it)
		{
			processedNodes.insert((*it)->getLabel());
		}
		for (auto it = chain2.cbegin(); it != chain2.cend(); ++it)
		{
			processedNodes.insert((*it)->getLabel());
		}

		if (stDist == chain1.size() + chain2.size() + 2)
		{
			//we cannot remove the cycle as it may affect the value of delta

			//calculate maximal cycle size
			size_t cycleSize = (chain1.size() + chain2.size() + 2) * 2;
			delta = cycleDelta(cycleSize);
			return false;
		}
		else if (stDist < chain1.size() + chain2.size() + 2)
		{
			//calculate cycle size
			size_t cycleSize = chain1.size() + chain2.size() + 2 + stDist;

			//remove cycle!
			graph->unmarkNodes();
			origin->mark();
			for (auto it = chain1.begin(); it != chain1.cend(); ++it) (*it)->mark();
			for (auto it = chain2.begin(); it != chain2.cend(); ++it) (*it)->mark();
			graph->deleteMarkedNodes();

			delta = cycleDelta(cycleSize);
			return true;
		}
		else
		{
			//impossible - distance from s to t is at most the chains' length + 1 (origin)
			throw exception("Impossible distance received");
		}
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

	delta_t GraphAlgorithms::cycleDelta(size_t length)
	{
		unsigned int remainder = (length % 4);
		size_t p = length / 4;
		if (1 == remainder) return p-0.5;
		else return static_cast<delta_t>(p);
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
        node_ptr_collection_t furthestNodes;
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
		if (!edgeMarker) throw InvalidFormatException("Edge marker not found");
		const char* delimiter = strstr(edgeMarker + EdgeMarkerLen, Delimiter);
		if (delimiterPresent && !delimiter) throw InvalidFormatException("No delimiter found");
		
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
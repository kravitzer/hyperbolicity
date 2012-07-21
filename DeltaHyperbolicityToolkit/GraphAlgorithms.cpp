#include "GraphAlgorithms.h"
#include "Except.h"
#include "defs.h"
#include <memory>
#include <vector>
#include <algorithm>

using namespace std;

namespace graphs
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
		shared_ptr<FILE> outputFile = OpenFile(path.c_str(), "w");
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
		shared_ptr<FILE> inputFile = OpenFile(path.c_str(), "rb");
		
		//create nodes
		unsigned int nodeCount = ReadNodeCount(inputFile);
		graph_ptr_t g(new Graph);
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

	delta_t GraphAlgorithms::CalculateDelta(const graph_ptr_t graph, const node_quad_t& state)
	{
		node_collection_t nodeCollection;
		nodeCollection.push_back(state[1]);
		nodeCollection.push_back(state[2]);
		nodeCollection.push_back(state[3]);

		//calculate distances from v0 to v1, v2, v3
		distance_dict_t v0dists = GraphAlgorithms::Dijkstra(graph, state[0], nodeCollection);

		//calculate distances from v1 to v2, v3
		nodeCollection.erase(nodeCollection.begin());
		distance_dict_t v1dists = GraphAlgorithms::Dijkstra(graph, state[1], nodeCollection);

		//calculate distances from v2 to v3
		nodeCollection.erase(nodeCollection.begin());
		distance_dict_t v2dists = GraphAlgorithms::Dijkstra(graph, state[2], nodeCollection);

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
		distance_t& largest = d1, secondLargest = d1;
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

	distance_dict_t GraphAlgorithms::Dijkstra(const graph_ptr_t graph, const node_ptr_t origin, const node_collection_t& destination)
	{
		if (nullptr == origin.get()) throw InvalidParamException("Origin node is null");
		for (node_collection_t::const_iterator it = destination.cbegin(); it != destination.cend(); ++it)
		{
			if ( (nullptr == it->get()) || (!graph->hasNode(*it)) ) throw InvalidParamException("Destination node is invalid");
		}

		//create the distance vector and initizlize all distances to infinity
		distance_dict_t distance;
		for (unsigned int i = 0; i < graph->size(); ++i)
		{
			distance[i] = InfiniteDistance;
		}

		//origin node is at distance 0, obviously
		distance[origin->getIndex()] = 0;

		//create the collection of nodes remaining to process (all nodes in the graph, at this point)
		distance_dict_t remainingNodes(graph->size());
		for (unsigned int i = 0; i < graph->size(); ++i)
		{
			remainingNodes[i] = i;
		}

		//run loop until there are no more nodes left, or until we found the nodes we're looking for
		unsigned int nodesFound = 0;
		while ( (remainingNodes.size() > 0) && (nodesFound < destination.size()) )
		{
			//in the remaining node collection, find the node whose distance to the origin is the smallest
			node_index_t shortestDistanceNode = (remainingNodes.begin())->first;
			for (distance_dict_t::const_iterator it = remainingNodes.cbegin(); it != remainingNodes.cend(); ++it)
			{
				//if current node's distance is infinity, then there's no way it is less than the currently shortest path, so skip
				if (InfiniteDistance == distance[it->first]) continue;

				//if current shortest path is infinity, or if the current node has a shorter path, update the shortest path
				if ( (InfiniteDistance == distance[shortestDistanceNode]) || (distance[it->first] < distance[shortestDistanceNode]) ) shortestDistanceNode = it->first;
			}

			//if all nodes are infinitely far from the origin, we're done, as we won't make any progress
			if (InfiniteDistance == distance[shortestDistanceNode]) break;

			//remove the currently processed node from the remaining node collection
			remainingNodes.erase(shortestDistanceNode);

			//iterate through node's neighbors
			node_collection_t neighbors = graph->getNode(shortestDistanceNode)->getEdges();
			for (node_collection_t::const_iterator it = neighbors.cbegin(); it != neighbors.cend(); ++it)
			{
				node_index_t curNode = (*it)->getIndex();

				//skip neighbors that are not in the reamining node collection, as we've already found a shorter path to them
				if (remainingNodes.end() == remainingNodes.find(curNode)) continue;

				distance_t curNodeDistance = distance[curNode];
				unsigned int pathLen = distance[shortestDistanceNode] + 1;
				if ( (curNodeDistance == InfiniteDistance) || (static_cast<distance_t>(pathLen) < curNodeDistance) )
				{
					distance[curNode] = pathLen;
					//increment the count of nodes found if this is one of the nodes we were looking for
					if (find(destination.cbegin(), destination.cend(), *it) != destination.cend()) ++nodesFound;
				}

				//exit loop if we found all the nodes we were looking for
				if (destination.size() == nodesFound) break;
			}
		}

		//build the distance dictionary for the requested nodes only
		if (destination.size() == distance.size()) return distance;
		distance_dict_t ret;
		for (node_collection_t::const_iterator it = destination.cbegin(); it != destination.cend(); ++it)
		{
			ret[(*it)->getIndex()] = distance[(*it)->getIndex()];
		}
		return ret;
	}

	distance_dict_t GraphAlgorithms::Dijkstra(const graph_ptr_t graph, const node_ptr_t origin)
	{
		node_collection_t graphNodes(graph->size());
		for (unsigned int i = 0; i < graph->size(); ++i) graphNodes[i] = graph->getNode(i);
		return Dijkstra(graph, origin, graphNodes);
	}

	distance_t GraphAlgorithms::Dijkstra(const graph_ptr_t graph, const node_ptr_t origin, const node_ptr_t destination)
	{
		node_collection_t nodeCollection(1);
		nodeCollection[0] = destination;
		return Dijkstra(graph, origin, nodeCollection)[destination->getIndex()];
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
		node_ptr_t firstSweepNode = Sweep(graph, startNode, nullptr, nullptr);
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

	void GraphAlgorithms::PruneTrees(graph_ptr_t graph)
	{
		for (unsigned int i = 0; i < graph->size(); ++i)
		{
			node_ptr_t node = graph->getNode(i);

			//check if current node needs to be pruned
			if (node->getEdges().size() <= 1)
			{
				//prune node recursively
				i -= pruneTreesRecursion(graph, node, node);
			}
		}
	}

	unsigned int GraphAlgorithms::pruneTreesRecursion(graph_ptr_t graph, node_ptr_t curNode, node_ptr_t originalNode)
	{
		node_collection_t edges = curNode->getEdges();
		size_t edgesLen = edges.size();
		unsigned int nodesRemoved = 0;

		//assert degree is <= 1
		if (edgesLen > 1) throw std::exception("Pruned node degree is > 1");
		
		//if node has a neighbor (can have only 1, if any), remember it before we remove the edge
		node_ptr_t neighbor(nullptr);
		if (1 == edgesLen) neighbor = edges[0];

		//prune node from graph, and add 1 to node removal count if it is <= the original node (i.e. comes before it
		//in the graph's node enumeration).
		graph->removeNode(curNode);
		if (curNode->getIndex() <= originalNode->getIndex()) ++nodesRemoved;

		//if the pruned node had a neighbor, and it has now become a leaf, prune it recursively
		if ( (nullptr != neighbor.get()) && (neighbor->getEdges().size() <= 1) ) nodesRemoved += pruneTreesRecursion(graph, neighbor, originalNode);

		//the number returned here is the number of nodes we need to go back in the node iteration of the graph (since this
		//many nodes have been removed prior to the original node)
		return nodesRemoved;
	}

	node_ptr_t GraphAlgorithms::Sweep(const graph_ptr_t graph, const node_ptr_t origin, distance_t* dist, distance_dict_t* distancesFromU)
	{
		distance_dict_t startNodeDistances = Dijkstra(graph, origin);
		if (distancesFromU) *distancesFromU = startNodeDistances;

		//find the maximal distance
		distance_t maxDistance = 0;
		for (distance_dict_t::const_iterator it = startNodeDistances.cbegin(); it != startNodeDistances.cend(); ++it)
		{
			if (InfiniteDistance == it->second) continue;
			if (it->second > maxDistance) maxDistance = it->second;
		}

		//get the nodes that are furthest from the origin (i.e. maxDistance away)
		node_collection_t furthestNode;
		for (distance_dict_t::const_iterator it = startNodeDistances.cbegin(); it != startNodeDistances.cend(); ++it)
		{
			if (maxDistance == it->second) furthestNode.push_back(graph->getNode(it->first));
		}

		//randomly select node out of the furthest nodes
		unsigned int selectedNodeIndex = rand() % furthestNode.size();
		if (dist) *dist = maxDistance;
		return furthestNode[selectedNodeIndex];
	}

	shared_ptr<FILE> GraphAlgorithms::OpenFile(const char* path, const char* mode)
	{
		FILE* outputFile = nullptr;
		if (0 != fopen_s(&outputFile, path, mode))
		{
			throw exception("Failed opening file");
		}
		return shared_ptr<FILE>(outputFile, &fclose);
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
			nodeIndexLen = sprintf_s(nodeIndexString, "%s%s", nodeIndex, Delimiter);
		}
		else
		{
			nodeIndexLen = sprintf_s(nodeIndexString, "%s, ", nodeIndex);
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
		while (s[i] != '\0' && p[i] != '\0')
		{
			if (s[i] != p[i]) return false;
			++i;
		}

		return true;
	}
} // namespace graphs
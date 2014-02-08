/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_GRAPH_ALGORITHMS_H
#define DELTAHYPER_GRAPH_ALGORITHMS_H

#include "DeltaHyperbolicity.h"
#include "Graph.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace dhtoolkit
{
	/*
	 * @brief	A collection of some useful graph methods.
	 */
	class GraphAlgorithms
	{
	public:
		struct DoubleSweepResult
		{
			node_ptr_t u;
			node_ptr_t v;
			distance_t dist;
			distance_dict_t uDistances;
		};

		/*
		 * @brief	Saves the given graph to a file.
		 * @param	graph	The graph to be saved.
		 * @param	path	The file path where the graph is to be stored.
		 * @throws	std::exception	Upon an I/O failure or if graph is empty.
		 * @note	The method will overwrite any previously existing file in the given path.
		 */
		static void SaveGraphToFile(const graph_ptr_t graph, const std::string& path);

		/*
		 * @brief	Loads the graph from the given file.
		 * @param	path	The file path to be loaded.
		 * @returns	A graph instance loaded from the file.
		 * @throws	std::exception	Upon a failure (I/O failure, invalid format, etc.).
		 */
		static graph_ptr_t LoadGraphFromFile(const std::string& path);


		/*
		 * @brief	Loads the graph from a given file, where each line is of the form: <node-id1>\t<node-id2>\n
		 *			Comments are allowed if line starts with "#".
		 * @param	path			The file path to be loaded.
		 * @param	isBidirectional	True if each line represents a bidirectional edge, false otherwise.
		 * @returns	A graph instance loaded from the file.
		 * @throws	std::exception	Upon a failure (I/O failure, invalid format, etc.).
		 */
		static graph_ptr_t LoadGraphFromEdgeListFile(const std::string& path, bool isBidirectional);

		/*
		 * @brief	Creates a Pajek net file that can be visualized using Pajek.
		 * @param	fileName		The name of the files to be created.
		 * @param	graph			The graph to be drawn.
		 * @param	nodesToMark		Nodes in this parameter will be given a unique color.
		 * @throws	std::exception	Upon any error.
		 */
		static void GraphAlgorithms::drawGraph(const std::string& fileName, const graph_ptr_t graph, const node_combination_t* nodesToMark);

		/*
		 * @brief	Calculates the delta value of the given state nodes.
		 * @param	graph	The graph to calculate delta for.
		 * @param	state	The state for which we need to calculate the delta value.
		 * @returns	The calculated delta hyperbolicity value.
		 */
		static delta_t CalculateDelta(const graph_ptr_t graph, const node_combination_t& state);

		/*
		 * @brief	Delta calculation considers all possible distances between 4 nodes (3 pairs of distances).
		 *			E.g. d1 = dist(v1, v2) + dist(v3, v4)
		 *				 d2 = dist(v1, v3) + dist(v2, v4)
		 *				 d3 = dist(v1, v4) + dist(v2, v3)
		 *			We then take the two largest numbers and the absolute value of their difference, over 2, is the delta hyperbolicity
		 *			for that node quad. This method calculates this value based on the 3 distance pairs.
		 * @returns	The delta obtained from the 3 distance pairs.
		 */
		static delta_t CalculateDeltaFromDistances(distance_t d1, distance_t d2, distance_t d3);
		/*
		 * @brief	Runs a double-sweep on the given node. If several nodes are "the furthest", selects one randomly (equally distributed).
		 * @param	graph	The graph to run on.
		 * @param	origin	The node from which the double-sweep process starts. Optional. If not specified, the origin node is randomly selected.
		 * @returns	A DoubleSweepResult structure, holding the result of this process.
		 * @note	IMPORTANT! You must initialize a random seed (by calling srand() with some random seed) before calling this method, otherwise
		 *			you might get the same results each run!
		 */
		static DoubleSweepResult DoubleSweep(const graph_ptr_t graph, const node_ptr_t origin = node_ptr_t(nullptr));

		/*
		 * @returns	A random state from the graph given.
		 */
		static node_combination_t getRandomState(const graph_ptr_t graph);

		/*
		 * @returns	Returns the collection of the sub-graphs that are biconnected within the graph.
		 */
		static graph_ptr_collection_t getBiconnectedComponents(const graph_ptr_t graph);

		/*
		 * @brief	Removes the cycle that origin is a part of, if really is a part of a cycle, and if cycle may be removed without
		 *			affecting the hyperbolicity.
		 *
		 *			The cycle removal algorithm is as follows:
		 *				Starting at the origin node v,  for each of v's neighbors u: as long as degree(u) == 2, go to the neighbor that you did 
		 *				not come from. Of course, if we've somehow reached the first node - the graph is a simple cycle and we're done...
		 *				We now reached two vertices, s & t, whose degree > 2. Check d(s, t).
		 *				If d(s, t) < number of nodes iterated + 1 (excluding s & t) then the nodes iterated (excluding s & t) may be removed!
		 *				Before removal, we calculate the delta value produced from the cycle (truncate(length / 4) if length % 4 != 1, 
		 *				or truncate(length / 4) - 0.5 otherwise).
		 *				If d(s, t) = number of nodes iterated + 1 (excluding s & t), then unfortunatelly we cannot remove the cycle as it may
		 *				affect other distances in the graph. We simply calculate the delta of the cycle, assuming that the shortest path from s to
		 *				t that does not go through the chain of nodes iterated is of the same length (this yields a delta that cannot be larger than
		 *				the real value).
		 * @param	graph	The graph to run on.
		 * @param	origin	The node to start from.
		 * @param	delta	Will be set to the delta of the cycle found.
		 * @param	processedNodes		In case the cycle cannot be removed, the labels of the nodes processed will be added to this set (the user 
		 *								may keep them in order to avoid running on them in the future).
		 *								In case a cycle was removed, the labels of the nodes revmoed.
		 * @returns	Whether a cycle was removed from the graph or not.
		 */
		static bool removeCycle(graph_ptr_t graph, node_ptr_t origin, delta_t& delta, std::unordered_set<std::string>& processedNodes);

		/*
 		 * @brief	Prunes trees from the graph. I.e. removes all nodes with degree 0 or 1 (and those that become of this degree as a result
		 *			of these deletions, recursively).
		 * @param	graph	The graph to run on.
		 */
		static void pruneTrees(graph_ptr_t graph);

	private:
		static const int NodeIndexMaxNumOfDigits;
		static const char* EdgeMarker;
		static const size_t EdgeMarkerLen;
		static const int EdgeMaxLen;
		static const char* Delimiter;
		static const size_t DelimiterLen;
		static const char PathSeparator;

		struct Edge
		{
			node_index_t src;
			node_index_t dst;
		};
		
		/*
		 * @param node	The node for which to perform the count.
		 * @returns The number of outgoing edges from the given node, pointing to unmarked nodes.
		 */
		static unsigned int countUnmarkedNeighbors(node_ptr_t node);

		/*
		 * @brief	Marks the given current leaf node and recursively calls itself on its neighbor (if exists).
		 * @param	curNode			The current node to mark for deletion. Must have degree 0 or 1.
		 * @returns The number of nodes marked for deletion.
		 * @throws	std::exception	If the current node's degree is greater than 1.
		 */
		static unsigned int pruneTreesRecursion(node_ptr_t curNode);

		/*
		 * @returns The first unmarked neighbor of the node given.
		 * @throws  std::exception if there is none.
		 */
		static node_ptr_t getUnmarkedNeighbor(const node_ptr_t node);

		/*
		 * @returns	Shortens the path given such that it contains only the last three parts (e.g. "C:\dir1\dir2\dir3\dir4\dir5\file.txt" --> "dir4\dir5\file.txt").
		 *			If there are less than three parts, return the same string given (e.g. "C:\dir1\file.txt" --> "C:\dir1\file.txt").
		 */
		static std::string shortPath(const std::string& path);

		/*
		 * @returns	The delta value of a cycle-graph whose length is given.
		 */
		static delta_t cycleDelta(size_t length);

		static void biconnected(const graph_ptr_t graph, node_index_t v, node_index_t u, std::unordered_map<node_index_t, unsigned int>& number, std::unordered_map<node_index_t, unsigned int>& lowpt, unsigned int index, std::vector<std::pair<node_index_t, node_index_t>>& edgeStack, graph_ptr_collection_t& biconnectedGraphs);


		/*
		 * @param	node			The node to be checked.
		 * @param	nodesToMark		The collection of nodes to be marked. May be null (function will return false).
		 * @returns	True if node is to be marked, false otherwise.
		 */
		static bool GraphAlgorithms::isNodeToBeMarked(node_ptr_t node, const node_combination_t* nodesToMark);

		/*
		 * @brief Performs a single sweep on the graph - i.e. takes the origin node and randomly selects one of the nodes furthest away from it.
		 * @param	graph			The graph to run on.
		 * @param	origin			The node to perform the sweep from.
		 * @param	dist			Optional. Will be set to the distance of the node returned from the original node.
		 * @param	distancesFromU	Optional. Will be set to the distance collection from the origin node.
		 * @returns	The node selected from the sweep process.
		 */
		static node_ptr_t Sweep(const graph_ptr_t graph, const node_ptr_t origin, distance_t* dist, distance_dict_t* distancesFromU);

		/*
		 * @brief	Opens the given file.
		 * @returns	A smart pointer to the file instance. File will be closed when pointer is out of scope.
		 * @throws	std::exception upon I/O error.
		 */
		static std::shared_ptr<FILE> OpenFile(const char* path, const char* mode, int share);

		/*
		 * @brief	Writes the given node index into the given file.
		 * @param	filePtr		The file instance to write into.
		 * @param	nodeIndex	The node index to write.
		 * @param	isLast		If false, adds a comma and a space after the node index (e.g. "3, " for node index 3).
		 *						If true, adds a newline character after the node index (e.g. "3\n" for node index 3).
		 * @throws	std::exception	Upon an error.
		 */
		static void WriteNodeToFile(std::shared_ptr<FILE> filePtr, node_index_t nodeIndex, bool isLast);

		/*
		 * @brief	Writes the given edge into the given file.
		 * @param	filePtr		The file instance to write into.
		 * @param	srcIndex	The index of the source node.
		 * @param	dstIndex	The index of the destination node.
		 * @throws	std::exception	Upon an error.
		 */
		static void WriteEdgeToFile(std::shared_ptr<FILE> filePtr, node_index_t srcIndex, node_index_t dstIndex);

		/*
		 * @brief	Reads the nodes from the graph file.
		 * @param	filePtr		The file to read from.
		 * @returns	The number of nodes stored in the graph.
		 * @throws	std::excpetion	Upon an error.
		 */
		static unsigned int ReadNodeCount(std::shared_ptr<FILE> filePtr);

		/*
		 * @brief	Reads the edges from the garph file.
		 * @param	filePtr		The file to read from.
		 * @returns	A map of source and destination node indices.
		 * @throws	std::excpetion	Upon an error.
		 */
		static std::vector<Edge> ReadEdges(std::shared_ptr<FILE> filePtr);

		/*
		 * @brief	Reads a single edge from the given buffer.
		 * @param	buf					The buffer to read from.
		 * @param	delimiterPresent	Does a delimiter must appear? If true, and no delimiter is found,
		 *								throws an exception. Otherwise will read up to the end of the buffer.
		 * @param	dataReadFromBuffer	Optional output param - gets set to the number of bytes taken by the edge.
		 * @returns	The edge read.
		 * @throws	std::excetion Upon an error (e.g. invalid buffer - no edge marker is present).
		 */
		static Edge ReadSingleEdge(const char* buf, bool delimiterPresent, size_t* dataReadFromBuffer);

		/*
		 * @returns	True iff s starts with p or p starts with s.
		 */
		static bool StringStartsWith(const char* s, const char *p);
	};
} // namespace dhtoolkit

#endif
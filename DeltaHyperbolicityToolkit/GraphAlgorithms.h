#ifndef DELTAHYPER_GRAPH_ALGORITHMS_H
#define DELTAHYPER_GRAPH_ALGORITHMS_H

#include "DeltaHyperbolicity.h"
#include "Graph.h"
#include <vector>

namespace graphs
{
	class GraphAlgorithms
	{
	public:
		struct DoubleSweepResult
		{
			node_ptr_t u;
			node_ptr_t v;
			distance_t dist;
		};

		/*
		 * @brief	Saves the given graph to a file.
		 * @param	graph	The graph to be saved.
		 * @param	path	The file path where the graph is to be stored.
		 * @throws	std::exception	Upon an I/O failure.
		 * @note	The method will overwrite any previously existing file in the given path.
		 */
		static void SaveGraphToFile(const graph_ptr_t graph, const std::string& path);

		/*
		 * @brief	Loads the graph from the given file.
		 * @param	path	The file path to be loaded.
		 * @returns	A graph instance loaded from the file.
		 * @throws	InvalidFormatException	Upon an invalid file format.
		 * @throws	std::exception			Upon an I/O failure.
		 */
		static graph_ptr_t LoadGraphFromFile(const std::string& path);

		/*
		 * @brief	Calculates the delta value of the given state nodes.
		 * @param	graph	The graph to calculate delta for.
		 * @param	state	The state for which we need to calculate the delta value.
		 * @returns	The calculated delta hyperbolicity value.
		 */
		static delta_t CalculateDelta(const graph_ptr_t graph, const node_quad_t& state);

		/*
		 * @brief	Runs Dijkstra's algorithm, to find the distance between the given origin node and the various given
		 *			destination nodes.
		 * @param	graph	The graph to run on.
		 * @param	origin	The origin node from where the distances are measures.
		 * @param	destination	The destination nodes - upon finding the distances to all these nodes, 
		 *						the algorithm will stop & return.
		 * @returns	A dictionary where each of the destination nodes is mapped to its distance from the origin node.
		 * @throws	InvalidParamException	If any of the nodes is invalid (i.e. null) or if they are not part of the graph.
		 */
		static distance_dict_t Dijkstra(const graph_ptr_t graph, const node_ptr_t origin, const node_collection_t& destination);

		/*
		 * @brief	Runs Dijkstra's algorithm, starting from the given node.
		 * @param	graph	The graph to run on.
		 * @param	origin	The origin node from where the distances are measures.
		 * @returns	A dictionary where each node of the graph is mapped to its distance from the origin node.
		 * @throws	InvalidParamException	Upon an invalid node (null pointer) or if node is not part of the graph.
		 */
		static distance_dict_t Dijkstra(const graph_ptr_t graph, const node_ptr_t origin);

		/*
		 * @brief	Runs Dijkstra's algorithm, to find the distance between the given nodes.
		 * @param	graph	The graph to run on.
		 * @param	origin	The origin node from where the distances are measures.
		 * @param	destination	The destination node - upon finding the distance to this node the algorithm will stop & return.
		 * @returns	The distance between origin and destination nodes.
		 * @throws	InvalidParamException	If any of the nodes is invalid (i.e. null) or if they are not part of the graph.
		 */
		static distance_t Dijkstra(const graph_ptr_t graph, const node_ptr_t origin, const node_ptr_t destination);

		/*
		 * @brief	Runs a double-sweep on the given node. If several nodes are "the furthest", selects one randomly (equally distributed).
		 * @param	graph	The graph to run on.
		 * @param	origin	The node from which the double-sweep process starts. Optional. If not specified, the origin node is randomly selected.
		 * @returns	A DoubleSweepResult structure, holding the result of this process.
		 * @throws	InvalidParamException	If any of the nodes is invalid (i.e. null) or if they are not part of the graph.
		 */
		static DoubleSweepResult DoubleSweep(const graph_ptr_t graph, const node_ptr_t origin = node_ptr_t(nullptr));

		/*
		 * @brief	Prunes trees from the graph. I.e. removes all nodes with degree 0 or 1 (and those that become of this degree as a result
		 *			of these deletions, recursively).
		 */
		static void PruneTrees(graph_ptr_t graph);

	private:
		static const int NodeIndexMaxNumOfDigits;
		static const char* EdgeMarker;
		static const unsigned int EdgeMarkerLen;
		static const int EdgeMaxLen;
		static const char* Delimiter;
		static const unsigned int DelimiterLen;

		struct Edge
		{
			node_index_t src;
			node_index_t dst;
		};

		/*
		 * @brief	Removes the given current node and recursively calls itself on its neighbor (if exists).
		 * @param	graph			The graph to prune.
		 * @param	curNode			The current node to delete. Must have degree 0 or 1.
		 * @param	originalNode	The node the recursion started with (i.e. first call to this method will have 
		 *							both curNode and originalNode pointing at the same node).
		 * @returns	The number of nodes whose index is <= originalNode's index that were removed due to the pruning.
		 * @throws	std::exception	If the current node's degree is greater than 1.
		 */
		static unsigned int pruneTreesRecursion(graph_ptr_t graph, node_ptr_t curNode, node_ptr_t originalNode);

		/*
		 * @brief Performs a single sweep on the graph - i.e. takes the origin node and randomly selects one of the nodes furthest away from it.
		 * @param	graph	The graph to run on.
		 * @param	origin	The node to perform the sweep from.
		 * @param	dist	Optional. Will be set to the distance of the node returned from the original node.
		 * @returns	The node selected from the sweep process.
		 */
		static node_ptr_t Sweep(const graph_ptr_t graph, const node_ptr_t origin, distance_t* dist);

		/*
		 * @brief	Opens the given file.
		 * @returns	A smart pointer to the file instance. File will be closed when pointer is out of scope.
		 * @throws	std::exception upon I/O error.
		 */
		static std::shared_ptr<FILE> OpenFile(const char* path, const char* mode);

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
		static Edge ReadSingleEdge(const char* buf, bool delimiterPresent, unsigned int* dataReadFromBuffer);

		/*
		 * @returns	True iff s starts with p or p starts with s.
		 */
		static bool StringStartsWith(const char* s, const char *p);
	};
} // namespace graphs

#endif
/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#pragma once

#include "Graph.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace hyperbolicity
{
	/*
	 * @brief	A collection of some useful graph methods.
	 */
	class GraphAlgorithms
	{
	public:
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
		 * @returns	Returns the collection of the sub-graphs that are biconnected within the graph.
		 */
		static graph_ptr_collection_t getBiconnectedComponents(const graph_ptr_t graph);

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
		 * @returns	Shortens the path given such that it contains only the last three parts (e.g. "C:\dir1\dir2\dir3\dir4\dir5\file.txt" --> "dir4\dir5\file.txt").
		 *			If there are less than three parts, return the same string given (e.g. "C:\dir1\file.txt" --> "C:\dir1\file.txt").
		 */
		static std::string shortPath(const std::string& path);

		static void biconnected(const graph_ptr_t graph, node_index_t v, node_index_t u, std::unordered_map<node_index_t, unsigned int>& number, std::unordered_map<node_index_t, unsigned int>& lowpt, unsigned int index, std::vector<std::pair<node_index_t, node_index_t>>& edgeStack, graph_ptr_collection_t& biconnectedGraphs);
		
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
} // namespace hyperbolicity

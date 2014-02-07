/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_GRAPH_H
#define DELTAHYPER_GRAPH_H

#include "Node.h"
#include "defs.h"
#include <string>

namespace dhtoolkit
{

/*
 * @brief	Represents a graph, which is simply a collection of nodes (each node "knows" its edges)...
 */
class Graph
{
public:
	/*
	 * @brief	Default ctor. Nodes may be added using the insertion methods.
	 * @param	title	Graph title.
	 */
	Graph(const std::string& title);

	//move ctor, copy ctor & assignment operator
	Graph(const Graph& other);
	Graph(Graph&& other);
	Graph& operator=(Graph other);

	/*
	 * @returns	The graph title.
	 */
	std::string getTitle() const;

	/*
	 * @brief	Inserts a new node to the graph.
	 * @param	label	Node's label. If empty, it will be set to the node's index.
	 * @returns	A pointer to the newly created node.
	 */
	node_ptr_t insertNode(const std::string& label = std::string());

	/*
	 * @returns	The node whose index is given.
	 * @note	In favor of performance, this method does not verify input validity.
	 */
	node_ptr_t getNode(node_index_t index) const;

	/*
	 * @returns	The size of the graph (i.e. # of nodes).
	 */
	size_t size() const;

	/*
	 * @returns	The number of edges in the graph.
	 * @note	Bidirectional edges are counted twice!
	 */
	size_t edgeCount() const;

	/*
	 * @brief	Removes a node from the graph, based on the given index.
	 * @param	index	The index of the node to be removed.
	 * @note	In favor of performance, this method does not verify input validity.
	 * @note	This operation is very expensive - if you need to remove a large number of nodes, consider marking them and calling deleteMarkedNodes() instead.
	 */
	void removeNode(node_index_t index);

	/*
	 * @brief	Removes a node from the graph.
	 * @param	node	The node to be removed.
	 * @note	In favor of performance, this method does not verify input validity.
	 * @note	This operation is very expensive - if you need to remove a large number of nodes, consider marking them and calling deleteMarkedNodes() instead.
	 */
	void removeNode(node_ptr_t node);

	/*
	 * @brief	Unmarks all nodes in the graph.
	 */
	void unmarkNodes() const;

	/*
	 * @brief	Deletes all nodes that are currently marked from the graph. 
	 * @note	This method is much more efficient than calling removeNode() multiple times, when intending to remove a *large* number of nodes.
	 */
	void deleteMarkedNodes();

	/*
	 * @brief	Swaps the contents of the given graphs.
	 */
	friend void swap(Graph& first, Graph& second);

private:
    /*
     * @brief   Removes the node's incoming & outgoing edges (also removes the corresponding edges from its neighbors).
     */
    void removeNodeEdges(node_ptr_t node);

	/*
	 * @brief	Asserts that the given index represents a valid node (i.e. the index is valid in the nodes collection).
	 * @param	index				The index to assert.
	 * @throws	std::out_of_range	Upon an index that breaks the assertion.
	 */
	void assertIndexInBounds(node_index_t index) const;


	//---------- local variables ----------

	std::string _title;
	node_ptr_collection_t _nodes;
};

} // namespace dhtoolkit

#endif
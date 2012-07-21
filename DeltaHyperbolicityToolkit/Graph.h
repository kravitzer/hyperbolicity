/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_GRAPH_H
#define DELTAHYPER_GRAPH_H

#include "Node.h"
#include "defs.h"

namespace graphs
{

/*
 * @brief	Represents a graph, which is simply a collection of nodes (each node "knows" its edges)...
 */
class Graph
{
public:
	/*
	 * @brief	Default ctor. Nodes may be added using the insertion methods.
	 */
	Graph();

	/*
	 * @brief	Inserts a new node to the graph.
	 * @returns	A pointer to the newly created node.
	 */
	node_ptr_t insertNode();

	/*
	 * @returns	The node whose index is given.
	 * @throws	OutOfBoundsException	Upon an invalid index.
	 */
	node_ptr_t getNode(node_index_t index) const;

	/*
	 * @returns	The size of the graph (i.e. # of nodes).
	 */
	unsigned int size() const;

	/*
	 * @returns	The number of edges in the graph.
	 * @note	Bidirectional edges are counted twice!
	 */
	unsigned int edgeCount() const;

	/*
	 * @brief	Removes a node from the graph, based on the given index.
	 * @param	index	The index of the node to be removed.
	 * @throws	OutOfBoundsException	Upon an index that does not exist in the graph.
	 */
	void removeNode(node_index_t index);

	/*
	 * @brief	Removes a node from the graph.
	 * @param	node	The node to be removed.
	 * @throws	InvalidParamException	The node parameter is null or points to a node that is
	 *									not in the current graph instance.
	 */
	void removeNode(node_ptr_t node);

	/*
	 * @returns	True iff node is a node in this graph, false otherwise.
	 * @param	node	The node to check.
	 */
	bool hasNode(node_ptr_t node);

private:
	//do *not* allow copy ctor / assignment operator
	Graph(const Graph&);
	Graph& operator=(const Graph&);

	/*
	 * @brief	Asserts that the given index represents a valid node (i.e. the index is valid in the nodes collection).
	 * @param	index	The index to assert.
	 * @throws	OutOfBoundsException	Upon an index that breaks the assertion.
	 */
	void assertIndexInBounds(node_index_t index) const;

	node_collection_t _nodes;
};

} // namespace graphs

#endif
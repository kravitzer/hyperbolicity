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
	Graph(std::string title);

	//copy ctor & assignment operator
	Graph(const Graph& other);
	Graph& operator=(const Graph& other);

	/*
	 * @returns	The graph title.
	 */
	std::string getTitle() const;

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
	 * @brief	Unmarks all nodes in the graph.
	 */
	void unmarkNodes() const;

    /*
 	 * @brief	Prunes trees from the graph. I.e. removes all nodes with degree 0 or 1 (and those that become of this degree as a result
	 *			of these deletions, recursively).
	 */
    void pruneTrees();

private:
    /*
     * @brief   Removes the node's incoming & outgoing edges (also removes the corresponding edges from its neighbors).
     */
    void removeNodeEdges(node_ptr_t node);

    /*
     * @returns The first unmarked neighbor of the node given.
     * @throws  std::exception if there is none.
     */
    node_ptr_t getUnmarkedNeighbor(const node_ptr_t node) const;
    /*
     * @returns The number of edges pointing to unmarked nodes.
     */
    unsigned int countUnmarkedNeighbors(node_ptr_t node) const;

    /*
	 * @brief	Marks the given current leaf node and recursively calls itself on its neighbor (if exists).
	 * @param	curNode			The current node to mark for deletion. Must have degree 0 or 1.
     * @returns The number of nodes marked for deletion.
	 * @throws	std::exception	If the current node's degree is greater than 1.
	 */
    unsigned int pruneTreesRecursion(node_ptr_t curNode);

	/*
	 * @brief	Sets this graph to equal the other graph.
	 */
	void reset(const Graph& other);
	/*
	 * @brief	Asserts that the given index represents a valid node (i.e. the index is valid in the nodes collection).
	 * @param	index	The index to assert.
	 * @throws	OutOfBoundsException	Upon an index that breaks the assertion.
	 */
	void assertIndexInBounds(node_index_t index) const;

	std::string _title;
	node_collection_t _nodes;
};

} // namespace dhtoolkit

#endif
/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_NODE_H
#define DELTAHYPER_NODE_H

#include <memory>
#include "defs.h"

namespace graphs
{

class Graph;

/*
 * @brief	Represents a single graph node.
 */
class Node : public std::enable_shared_from_this<Node>
{
	friend Graph;
public:
	/*
	 * @returns	A read-only vecotr of the node's pointed to by this node.
	 */
	const node_collection_t& getEdges() const;

	/*
	 * @brief	Adds an edge from this instance's node to the given node.
	 * @param	otherNode	The node to which the edge is drawn.
	 * @note	The edge is bidirectional - e.g. otherNode will have a new edge to
	 *			this node as well.
	 * @throws	InvalidParamException	Upon null pointer or nodes not belonging to the same graph.
	 */
	void insertBidirectionalEdgeTo(node_ptr_t otherNode);

	/*
	 * @brief	Adds an edge from this instance's node to the given node.
	 * @param	otherNode	The node to which the edge is drawn.
	 * @note	The other node will not be changed as this is a unidirectional edge!
	 * @throws	InvalidParamException	Upon null pointer or nodes not belonging to the same graph.
	 */
	void insertUnidirectionalEdgeTo(const node_ptr_t otherNode);

	/*
	 * @brief	Removes the edge to the given node.
	 * @param	otherNode	The node that is the destination of the edge to be removed.
	 * @throws	std::exception	If the edge does not exist.
	 */
	void removeEdge(const node_ptr_t otherNode);

	/*
	 * @returns	The node's index.
	 */
	node_index_t getIndex() const;

private:
	//do *not* allow copy ctor / assignment operator
	Node(const Node&);
	Node& operator=(const Node&);

	/*
	 * @brief	Ctor receiving the node's index. A node may only be created by a Graph instace.
	 * @param	index	Node's index - must be unique in the graph!
	 */
	Node(node_index_t index);

	/*
	 * @brief	A node's index may need to change (if a node is removed, for example). This
	 *			methods allows such a change. It is private so that it may only be called from
	 *			within the instance or from a friend class (the graph).
	 * @param	newIndex	The index to be set.
	 */
	void setIndex(node_index_t newIndex);

	node_index_t _index;
	node_collection_t _outgoingEdges;
	node_collection_t _incomingEdges;
};

} // namespace graphs

#endif
/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_NODE_H
#define DELTAHYPER_NODE_H

#include <memory>
#include "defs.h"

namespace dhtoolkit
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
	 * @returns	A read-only vector of the node's pointed to by this node.
	 */
	const node_weak_ptr_collection_t& getEdges() const;

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
	 * @brief	Checks whether there's an edge to the given node.
	 * @param	otherNode	The node that is the destination of the edge searched.
	 * @returns	True if an edge exists to or from other node, false otherwise.
	 * @throws	InvalidParamException	Upon null pointer or nodes not belonging to the same graph.
	 */
	bool hasEdge(const node_ptr_t otherNode) const;

	/*
	 * @returns	The node's index.
	 */
	node_index_t getIndex() const;

	/*
	 * @returns	Whether or not the node is currently  marked.
	 */
	bool isMarked() const;

	/*
	 * @brief	Marks the current node.
	 */
	void mark() const;

	/*
	 * @brief	Unmarks the current node.
	 */
	void unmark() const;

	/*
	 * @returns	The node's label
	 */
	std::string getLabel() const;

private:
	//do *not* allow copy ctor / assignment operator
	Node(const Node&);
	Node& operator=(const Node&);

	/*
	 * @brief	Ctor receiving the node's index. A node may only be created by a Graph instace.
	 * @param	index	Node's index - must be unique in the graph!
	 */
	Node(node_index_t index, std::string label);

	/*
	 * @brief	A node's index may need to change (if a node is removed, for example). This
	 *			methods allows such a change. It is private so that it may only be called from
	 *			within the instance or from a friend class (the graph).
	 * @param	newIndex	The index to be set.
	 */
	void setIndex(node_index_t newIndex);

	//node's index (0-based)
	node_index_t _index;

	//collection of node's incoming & outgoing nodes. NOTE: must be a weak pointer, otherwise when freeing
	//a graph, the nodes won't be released, as they will have cyclic references to each other!
	node_weak_ptr_collection_t _outgoingEdges;
	node_weak_ptr_collection_t _incomingEdges;

	//node's label - doesn't change even when the index changes
	std::string _label;

	//is node marked or not
	mutable bool _isMarked;
};

} // namespace dhtoolkit

#endif
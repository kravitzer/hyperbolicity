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
	friend Graph;	//only a graph can create a new node instance (i.e. access its ctor)
public:
	/*
	 * @returns	A read-only collection of the nodes pointed to by the current node.
	 */
	const node_weak_ptr_collection_t& getEdges() const;

	/*
	 * @brief	Adds a bidirectional edge from this instance's node to the given node.
	 * @param	otherNode	The node to which the edge is drawn.
	 * @note	The edge is bidirectional - e.g. otherNode will have a new edge to this node as well.
	 * @throws	std::invalid_argument	Upon null pointer or nodes not belonging to the same graph.
	 */
	void insertBidirectionalEdgeTo(node_ptr_t otherNode);

	/*
	 * @brief	Adds an edge from this instance's node to the given node.
	 * @param	otherNode	The node to which the edge is drawn.
	 * @note	The other node will not be changed as this is a unidirectional edge!
	 * @throws	std::invalid_argument	Upon null pointer or nodes not belonging to the same graph.
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
	 * @param	otherNode	The destination node of the edge to be looked for.
	 * @returns	True if an edge exists to or from another node, false otherwise.
	 * @throws	std::invalid_argument	Upon null pointer or nodes not belonging to the same graph.
	 */
	bool hasEdge(const node_ptr_t otherNode) const;

	/*
	 * @returns	The node's index.
	 */
	node_index_t getIndex() const;

	/*
	 * @returns	Whether or not the node is currently marked.
	 */
	bool isMarked() const;

	/*
	 * @brief	Marks/unmarks the current node.
	 * @param	isMarked	Whether to set or unset the given node.
	 */
	void setMarked(bool isMarked) const;

	/*
	 * @returns	The node's label
	 */
	std::string getLabel() const;

private:
	//do *not* allow copy ctor / move ctor / assignment operator - as a node must be assigned to a graph, and we cannot have two identical nodes in the same graph
	//so copying has no practical meaning
	Node(const Node&);
	Node(const Node&&);
	Node& operator=(const Node&);

	/*
	 * @brief	Ctor for creating a new node.
	 * @param	index	Node's index.
	 * @param	label	The node's textual description.
	 */
	Node(node_index_t index, std::string label);

	/*
	 * @brief	Resets the index of the node (node indices must remain consecutive in a graph, therefore a node's index may change, 
	 *			if for example, another node was removed from the graph).
	 * @param	newIndex	The node's new index.
	 */
	void setIndex(node_index_t newIndex);

	//the node's index (0-based)
	node_index_t _index;

	//collection of the node's incoming & outgoing nodes. NOTE: must be a weak pointer, otherwise when freeing
	//a graph, the nodes won't be released, as they will have cyclic references to each other!
	node_weak_ptr_collection_t _outgoingEdges;
	node_weak_ptr_collection_t _incomingEdges;

	//the node's label - remains unchanged throughout the life of the instance
	std::string _label;

	//is the node marked or not
	mutable bool _isMarked;
};

} // namespace dhtoolkit

#endif
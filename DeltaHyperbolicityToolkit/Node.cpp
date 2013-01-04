#include "Node.h"
#include "Except.h"

namespace dhtoolkit
{
	Node::Node(node_index_t index) : _index(index), _outgoingEdges(), _incomingEdges(), _isMarked(false)
	{
		//empty
	}

	const node_collection_t& Node::getEdges() const
	{
		return _outgoingEdges;
	}

	void Node::insertBidirectionalEdgeTo(node_ptr_t otherNode)
	{
		insertUnidirectionalEdgeTo(otherNode);
		otherNode->insertUnidirectionalEdgeTo(this->shared_from_this());
	}

	void Node::insertUnidirectionalEdgeTo(const node_ptr_t otherNode)
	{
		if (nullptr == otherNode.get()) throw InvalidParamException("Trying to insert a unidirectional edge to a null node pointer");

		_outgoingEdges.push_back(otherNode);
		otherNode->_incomingEdges.push_back(this->shared_from_this());
	}

	void Node::removeEdge(const node_ptr_t otherNode)
	{
		//find the edge
		node_collection_t::const_iterator it;
		for (it = _outgoingEdges.cbegin(); ( (it != _outgoingEdges.cend()) && (it->get() != otherNode.get()) ); ++it)
		;

		//edge does not exist
		if (it == _outgoingEdges.cend()) throw std::exception("Failed to find edge to be removed");

		//find this node in the other node's incoming edge collection
		node_collection_t::iterator incomingEdgeIt;
		for (incomingEdgeIt = (*it)->_incomingEdges.begin(); ( (incomingEdgeIt != (*it)->_incomingEdges.end()) && (incomingEdgeIt->get() != this) ); ++incomingEdgeIt)
		;

		//make sure we've found ourselves in the other node's collection
		if (incomingEdgeIt == (*it)->_incomingEdges.end()) throw std::exception("Node is not found in other node's incoming edge collection");

		//remove self from other node's collection
		(*it)->_incomingEdges.erase(incomingEdgeIt);

		//remove edge from this node's edge collection
		_outgoingEdges.erase(it);
	}

	node_index_t Node::getIndex() const
	{
		return _index;
	}

	bool Node::isMarked() const
	{
		return _isMarked;
	}

	void Node::mark() const
	{
		_isMarked = true;
	}

	void Node::unmark() const
	{
		_isMarked = false;
	}

	void Node::setIndex(node_index_t newIndex)
	{
		_index = newIndex;
	}

} // namespace dhtoolkit

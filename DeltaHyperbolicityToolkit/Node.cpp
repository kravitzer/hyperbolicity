#include "Node.h"
#include "Except.h"
#include <string>

using namespace std;

namespace dhtoolkit
{
	Node::Node(node_index_t index, string label) : _index(index), _outgoingEdges(), _incomingEdges(), _isMarked(false), _label(label)
	{
		//empty
	}

	const node_weak_ptr_collection_t& Node::getEdges() const
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
		node_weak_ptr_collection_t::const_iterator it;
		for (it = _outgoingEdges.cbegin(); ( (it != _outgoingEdges.cend()) && (it->lock().get() != otherNode.get()) ); ++it)
		;

		//edge does not exist
		if (it == _outgoingEdges.cend()) throw std::exception("Failed to find edge to be removed");

		//find this node in the other node's incoming edge collection
		node_weak_ptr_collection_t::iterator incomingEdgeIt;
		for (incomingEdgeIt = it->lock()->_incomingEdges.begin(); ( (incomingEdgeIt != it->lock()->_incomingEdges.end()) && (incomingEdgeIt->lock().get() != this) ); ++incomingEdgeIt)
		;

		//make sure we've found ourselves in the other node's collection
		if (incomingEdgeIt == it->lock()->_incomingEdges.end()) throw std::exception("Node is not found in other node's incoming edge collection");

		//remove self from other node's collection
		it->lock()->_incomingEdges.erase(incomingEdgeIt);

		//remove edge from this node's edge collection
		_outgoingEdges.erase(it);
	}

	bool Node::hasEdge(const node_ptr_t otherNode) const
	{
		if (nullptr == otherNode.get()) throw InvalidParamException("Null pointer when checking if edge exists");
		
		for (node_weak_ptr_collection_t::const_iterator it = _incomingEdges.cbegin(); it != _incomingEdges.cend(); ++it)
		{
			if ((*it).lock()->getIndex() == otherNode->getIndex()) return true;
		}
		
		for (node_weak_ptr_collection_t::const_iterator it = _outgoingEdges.cbegin(); it != _outgoingEdges.cend(); ++it)
		{
			if ((*it).lock()->getIndex() == otherNode->getIndex()) return true;
		}

		return false;
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

	string Node::getLabel() const
	{
		return _label;
	}

	void Node::setIndex(node_index_t newIndex)
	{
		_index = newIndex;
	}

} // namespace dhtoolkit

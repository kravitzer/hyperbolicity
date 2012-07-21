#include "Graph.h"
#include "defs.h"
#include "Node.h"
#include "Except.h"

namespace graphs
{
	Graph::Graph() : _nodes()
	{
		//empty
	}

	node_ptr_t Graph::insertNode()
	{
		//create new node whose index is the next avilable number (0-based)
		node_ptr_t newNode(new Node(_nodes.size()));
		_nodes.push_back(newNode);

		return newNode;
	}

	node_ptr_t Graph::getNode(node_index_t index) const
	{
		assertIndexInBounds(index);
		return _nodes[index];
	}

	unsigned int Graph::size() const
	{
		return _nodes.size();
	}

	void Graph::removeNode(node_index_t index)
	{
		assertIndexInBounds(index);
		node_ptr_t node = *(_nodes.begin() + index);
		_nodes.erase(_nodes.begin()+index);

		//erase node's outgoing edges
		while (node->getEdges().size() > 0)
		{
			node->removeEdge(node->getEdges()[0]);
		}

		//erase node's incoming edges
		while (node->_incomingEdges.size() > 0)
		{
			node_ptr_t otherNode = node->_incomingEdges[0];
			otherNode->removeEdge(node);
		}

		//update indecis of the nodes following the current node (each is actually decremented by 1)
		node_index_t curIndex = index;
		for (node_collection_t::const_iterator it = _nodes.begin()+index; it != _nodes.end(); ++it)
		{
			(*it)->setIndex(curIndex++);
		}
	}

	void Graph::removeNode(node_ptr_t node)
	{
		//check for null pointer
		if (nullptr == node.get()) throw InvalidParamException("Trying to remove a null node pointer");

		//get node index and make sure it is in bounds, and really is a member of *this* graph
		node_index_t nodeIndex = (*node)._index;
		if ((nodeIndex >= _nodes.size()) || (node.get() != _nodes[(*node)._index].get()))
		{
			throw InvalidParamException("Given node to remove is not a member of this graph");
		}

		removeNode(nodeIndex);
	}

	bool Graph::hasNode(node_ptr_t node)
	{
		//if index is out of bounds, the node cannot be in this graph
		node_index_t index = node->getIndex();
		if (index >= this->size()) return false;

		//if it is in bounds, make sure it points to the same place the node in the same
		//index is the graph is pointing to
		return (this->getNode(index).get() == node.get());
	}

	void Graph::assertIndexInBounds(node_index_t index) const
	{
		//check assertion
		if (index >= _nodes.size())
		{
			//index out of bounds
			const int ErrMsgBufLen = 100;
			char errMsg[ErrMsgBufLen];
			//format error message
			if (-1 == sprintf_s(errMsg, ErrMsgBufLen, "Node index %d requested is out of bounds (# of items: %d)", index, _nodes.size()))
			{
				//formatting failed - throw a "simple" exception without the formatted details
				throw OutOfBoundsException("Node index requested is out of bounds");
			}
			
			//throw exception with formatted message
			throw OutOfBoundsException(errMsg);
		}
	}
}
#include "Graph.h"
#include "defs.h"
#include "Node.h"
#include "NodeHasher.h"
#include "Except.h"
#include <string>
#include <unordered_map>
#include <algorithm>
#include <boost/format.hpp>

using namespace std;

namespace dhtoolkit
{
	Graph::Graph(const string& title) : _nodes(), _title(title)
	{
		//empty
	}

	Graph::Graph(const Graph& other)
	{
		_nodes.clear();
		_title = other._title;

		//create as many nodes as the other graph has
		for (unsigned int i = 0; i < other.size(); ++i)
		{
			node_ptr_t curNode = insertNode(other._nodes[i]->getLabel());
		}

		//create edges according to other graph
		for (node_ptr_collection_t::const_iterator it = other._nodes.cbegin(); it != other._nodes.cend(); ++it)
		{
			//current node in our graph
			node_ptr_t curNode = getNode((*it)->getIndex());
			//its edges in other graph
			const node_weak_ptr_collection_t& edges = (*it)->getEdges();
			//for each edge in other graph, create one here as well
			for (node_weak_ptr_collection_t::const_iterator edgeIt = edges.cbegin(); edgeIt != edges.cend(); ++edgeIt)
			{
				curNode->insertUnidirectionalEdgeTo( getNode( edgeIt->lock()->getIndex() ) );
			}
		}
	}

	Graph::Graph(Graph&& other) : _nodes(), _title()
	{
		swap(*this, other);
	}

	Graph& Graph::operator=(Graph other)
	{
		swap(*this, other);

		return *this;
	}

	string Graph::getTitle() const
	{
		return _title;
	}

	node_ptr_t Graph::insertNode(const string& label)
	{
		node_ptr_t newNode = nullptr;
		if (!label.empty())
		{
			//use given parameter as label
			newNode.reset(new Node(_nodes.size(), label));
		}
		else
		{
			//create a label that's simply the index of the node about to be created
			string indexLabel = (boost::format("%1%") % _nodes.size()).str();
			newNode.reset(new Node(_nodes.size(), indexLabel));
		}
		_nodes.push_back(newNode);

		return newNode;
	}

	node_ptr_t Graph::getNode(node_index_t index) const
	{
		//uncommenting the next line is safer, but also slower... don't write bugs and you'll be fine!
		//assertIndexInBounds(index);
		return _nodes[index];
	}

	size_t Graph::size() const
	{
		return _nodes.size();
	}

	size_t Graph::edgeCount() const
	{
		size_t edgeCount = 0;
		for (unsigned int i = 0; i < size(); ++i)
		{
			edgeCount += getNode(i)->getEdges().size();
		}

		return edgeCount;
	}


	void Graph::removeNode(node_index_t index)
	{
		//assert input validity (here we do prefer safety over performance, since this method shouldn't be called very often, and
		//even if it is - removing a node is already very expensive - this additional check is fairly negligible)
		assertIndexInBounds(index);

		node_ptr_collection_t::const_iterator nodeIt = (_nodes.begin() + index);
		node_ptr_t node = *nodeIt;
		_nodes.erase(nodeIt);

		removeNodeEdges(node);

		//update indecis of the nodes following the current node (each is actually decremented by 1)
		node_index_t curIndex = index;
		for (node_ptr_collection_t::const_iterator it = _nodes.begin()+index; it != _nodes.end(); ++it)
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

	void Graph::unmarkNodes() const
	{
		for (node_ptr_collection_t::const_iterator it = _nodes.cbegin(); it != _nodes.cend(); ++it)
		{
			(*it)->setMarked(false);
		}
    }

	void Graph::deleteMarkedNodes()
	{
        node_ptr_collection_t newCollection;
        unsigned int index = 0;

		//iterate over the nodes and insert only unmarked nodes to new collection
        for (node_ptr_collection_t::const_iterator it = _nodes.cbegin(); it != _nodes.cend(); ++it)
        {
            //check if node is marked for deletion or not
            if (!(*it)->isMarked())
            {
                //not marked - copy to new collection, and set index accordingly
				newCollection.push_back(*it);
				(*it)->setIndex(index++);
            }
            else
            {
                //marked for deletion - remove its incoming & outgoing edges (and obviously, do *not* add to new collection)
                removeNodeEdges(*it);
            }
        }

        //store new collection in class instance
        _nodes.swap(newCollection);
	}

    void Graph::removeNodeEdges(node_ptr_t node)
    {
        //erase node's outgoing edges
        while (node->getEdges().size() > 0)
        {
            node->removeEdge(node->getEdges()[0].lock());
        }

        //erase node's incoming edges
        while (node->_incomingEdges.size() > 0)
        {
            node_ptr_t otherNode = node->_incomingEdges[0].lock();
            otherNode->removeEdge(node);
        }
    }

	void Graph::assertIndexInBounds(node_index_t index) const
	{
		if (index >= _nodes.size())
		{
			//throw exception with formatted message
			throw OutOfBoundsException((boost::format("Node index %1% requested is out of bounds (# of nodes in graph: %2%)") % index % _nodes.size()).str().c_str());
		}
	}

	void swap(Graph& first, Graph& second)
	{
		//enable ADL
		using std::swap;

		swap(first._title, second._title);
		swap(first._nodes, second._nodes);
	}
}
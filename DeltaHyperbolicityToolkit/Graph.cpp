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

	node_ptr_t Graph::insertNode(string label)
	{
		//create new node whose index is the next avilable number (0-based)
		if (label.empty()) label = (boost::format("%1%") % _nodes.size()).str();
		node_ptr_t newNode(new Node(_nodes.size(), label));
		_nodes.push_back(newNode);

		return newNode;
	}

	node_ptr_t Graph::getNode(node_index_t index) const
	{
		assertIndexInBounds(index);
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
		assertIndexInBounds(index);
		node_ptr_t node = *(_nodes.begin() + index);
		_nodes.erase(_nodes.begin()+index);

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
			(*it)->unmark();
		}
    }

	void Graph::deleteMarkedNodes()
	{
        node_ptr_collection_t newCollection;
        unsigned int index = 0;
        for (node_ptr_collection_t::const_iterator it = _nodes.cbegin(); it != _nodes.cend(); ++it)
        {
            //check if node is marked for deletion or not
            if (!(*it)->isMarked())
            {
                //not marked - copy to new array, and set index
				newCollection.push_back(*it);
				(*it)->setIndex(index++);
            }
            else
            {
                //marked for deletion - remove its incoming & outgoing edges
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

	void Graph::pruneTrees()
    {
        //unmark all nodes
        unmarkNodes();

        unsigned int removed = 0;

        //run on all nodes, mark the ones that need to be removed
        for (node_ptr_collection_t::const_iterator it = _nodes.cbegin(); it != _nodes.cend(); ++it)
        {
            //check if current node needs to be pruned & not already pruned
            if ( (countUnmarkedNeighbors(*it) <= 1) && (!(*it)->isMarked()) )
            {
                //prune node recursively
                removed += pruneTreesRecursion(*it);
            }
        }

        //instead of deleting a ton of unnecessary nodes (deletion is VERY expensive!), we create
        //a new collection and copy only the necessary nodes into it, while setting their index appropriately
        //tests: using deletion, pruning of a large graph (1M+ nodes) took ~310 seconds. with this method - 0.238 seconds!!!

        deleteMarkedNodes();
    }

    unsigned int Graph::pruneTreesRecursion(node_ptr_t curNode)
    {
        unsigned int edgesLen = countUnmarkedNeighbors(curNode);
        
        //assert degree is <= 1
        if (edgesLen > 1) throw std::exception("Pruned node degree is > 1");

        //if node has a neighbor (can have only 1, if any), remember it before we remove the edge
        node_ptr_t neighbor(nullptr);
        if (1 == edgesLen) neighbor = getUnmarkedNeighbor(curNode);

        //mark node for pruning
        curNode->mark();
        unsigned int marked = 1;

        //if the pruned node had a neighbor, and it has now become a leaf, prune it recursively
        if ( (nullptr != neighbor.get()) && (countUnmarkedNeighbors(neighbor)) <= 1) marked += pruneTreesRecursion(neighbor);

        return marked;
    }

    node_ptr_t Graph::getUnmarkedNeighbor(const node_ptr_t node) const
    {
        const node_weak_ptr_collection_t& neighbors = node->getEdges();
        for (node_weak_ptr_collection_t::const_iterator it = neighbors.cbegin(); it != neighbors.cend(); ++it)
        {
            if (!it->lock()->isMarked()) return it->lock();
        }

        throw std::exception("No unmarked neighbor found!");
    }

    unsigned int Graph::countUnmarkedNeighbors(const node_ptr_t node) const
    {
        unsigned int unmarkedNodes = 0;
        const node_weak_ptr_collection_t& neighbors = node->getEdges();
        for (node_weak_ptr_collection_t::const_iterator it = neighbors.cbegin(); it != neighbors.cend(); ++it)
        {
            if (!it->lock()->isMarked()) ++unmarkedNodes;
        }

        return unmarkedNodes;
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

	void swap(Graph& first, Graph& second)
	{
		//enable ADL
		using std::swap;

		swap(first._title, second._title);
		swap(first._nodes, second._nodes);
	}
}
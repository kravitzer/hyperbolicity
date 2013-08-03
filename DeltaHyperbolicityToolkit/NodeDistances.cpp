#include "NodeDistances.h"
#include "Graph.h"
#include "Node.h"

namespace dhtoolkit
{

NodeDistances::NodeDistances(const graph_ptr_t graph, const node_ptr_t origin) : BFS(graph), _origin(origin), _runStarted(false)
{
	_distances[_origin->getIndex()] = 0;
}

NodeDistances::~NodeDistances()
{
	//empty on purpose
}

distance_dict_t NodeDistances::getDistances()
{
    startTraversalForNodes(node_ptr_collection_t());

	//add nodes that are not connected to origin with infinite distance
	for (unsigned int i = 0; i < _graph->size(); ++i)
	{
		if ( _distances.find(i) == _distances.end() ) _distances[i] = InfiniteDistance;
	}
    
    //by this point, the run was completed and all nodes were found
	return _distances;
}

distance_dict_t NodeDistances::getDistances(const node_ptr_collection_t& dests)
{
    if (dests.empty()) return distance_dict_t();

    startTraversalForNodes(dests);
    distance_dict_t ret;
    for (node_ptr_collection_t::const_iterator it = dests.cbegin(); it != dests.cend(); ++it)
    {
		if ( _distances.find((*it)->getIndex()) == _distances.end() )
		{
			ret[(*it)->getIndex()] = InfiniteDistance;
		}
		else
		{
			ret[(*it)->getIndex()] = _distances[(*it)->getIndex()];
		}
    }

    return ret;
}

distance_t NodeDistances::getDistance(node_ptr_t dest)
{
    node_ptr_collection_t singleNodeCollection;
    singleNodeCollection.push_back(dest);
    startTraversalForNodes(singleNodeCollection);
    return _distances[dest->getIndex()];
}

distance_t NodeDistances::getDistance(node_index_t index)
{
    return getDistance(_graph->getNode(index));
}

bool NodeDistances::nodeTraversal(const node_ptr_t curNode, const node_ptr_t prevNode, distance_t distanceFromOrigin)
{
	_distances[curNode->getIndex()] = distanceFromOrigin;
	
    if (_dests.empty())
    {
        //keep running until complete
        return true;
    }
    else
    {
        //if current node is one of those we're looking for...
        if (find(_dests.begin(), _dests.end(), curNode) != _dests.end())
        {
            //increment count of nodes found
            ++_nodesFound;
        }

        //if we've found them all, stop traversal (i.e. return false)
        return !(_nodesFound == _dests.size());
    }
}

void NodeDistances::startTraversalForNodes(const node_ptr_collection_t& dests)
{
    if (!_runStarted)
    {
        //run was never started
        _dests = dests;
        _runStarted = true;
        _nodesFound = 0;
        run(_origin);
    }
    else if (!isComplete())
    {
        //run was started but not completed

        //keep running until finding all nodes
        _dests.clear();
        if (dests.empty())
        {
            //run until finding all nodes
            resume();
        }
        else
        {
            //find which nodes are still to be found
            for (node_ptr_collection_t::const_iterator it = dests.cbegin(); it != dests.cend(); ++it)
            {
                if ( _distances.end() == _distances.find( (*it)->getIndex() ) )
                {
                    _dests.push_back(*it);
                }
            }

            //if all nodes are already found, return
            if (_dests.empty()) return; 
            
            //otherwise keep running until all are found
            _nodesFound = 0;
            resume();
        }
    }
}

} // namespace dhtoolkit
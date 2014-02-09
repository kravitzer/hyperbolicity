#include "BFS.h"
#include "defs.h"
#include "Graph.h"

using namespace std;

namespace hyperbolicity
{

BFS::BFS(graph_ptr_t graph) : _graph(graph), _isCompleted(false)
{
	//empty on purpose
}

BFS::~BFS()
{
	//empty on purpose
}

void BFS::run(const node_ptr_t origin)
{
	//create first BFS entry - the origin itself
	bfs_entry_t originEntry = {origin, nullptr, 0};
    std::queue<bfs_entry_t>().swap(_bfsQueue);
	_bfsQueue.push(originEntry);

	//unmark all nodes except for origin
	_graph->unmarkNodes();
	_graph->getNode( origin->getIndex() )->setMarked(true);

    //start running BFS
	runBfsLoop();
}

void BFS::run(node_index_t index)
{
	run( _graph->getNode(index) );
}

bool BFS::isComplete() const
{
    return _isCompleted;
}

void BFS::resume()
{
    if (_bfsQueue.empty()) throw std::exception("Cannot resume run (either already completed, or never started)");
    runBfsLoop();
}

void BFS::runBfsLoop()
{
    //stop flag
    bool keepRunning = true;

    //run BFS loop
    while (keepRunning && !_bfsQueue.empty())
    {
        //get first node in queue
        bfs_entry_t entry = _bfsQueue.front();
        _bfsQueue.pop();

		//call derived class implementation
        keepRunning = nodeTraversal(entry.node, entry.prevNode, entry.distance);

        //iterate through neighbors
        const node_weak_ptr_collection_t& neighbors = entry.node->getEdges();
        for (node_weak_ptr_collection_t::const_iterator it = neighbors.cbegin(); it != neighbors.cend(); ++it)
        {
            //skip if neighbor has already been marked
            if (it->lock()->isMarked()) continue;

            //mark neighbor
            it->lock()->setMarked(true);
            //add neighbor to queue
            bfs_entry_t newEntry = {it->lock(), entry.node, entry.distance+1};
            _bfsQueue.push(newEntry);
        }
    }

    //if we've arrived here with an empty queue, the run has been completed
    _isCompleted = _bfsQueue.empty();
}

} // namespace hyperbolicity
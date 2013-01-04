/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_BFS_H
#define DELTAHYPER_BFS_H

#include "defs.h"
#include <queue>

namespace dhtoolkit
{

/*
 * @brief   This class serves as a base class for BFS-based algorithms. It traverses the graph using breadth-first-search algorithm, and calls
 *          a pure virtual function on every edge. Derived classes may use it for search, or any other BFS-based operation.
 */
class BFS
{
public:
    /*
     * @brief   Ctor receiving the graph to run on.
     */
    BFS(const graph_ptr_t graph);

    /*
     * @brief   Default virtual dtor.
     */
    virtual ~BFS();

protected:
    /*
     * @brief   Starts the BFS algorithm from the origin node specified.
     * @throws  std::exception if node is not in current graph.
     */
    void run(const node_ptr_t origin);

    /*
     * @brief   An overload that takes a node index.
     */
    void run(node_index_t nodeIndex);

    /*
     * @brief   If a previous run has been stopped (i.e. nodeTraversal() returned false), this method resumes the run from the point
     *          it was stopped.
     * @NOTE    Assumes that the marking of the nodes hasn't changed!
     * @throws  std::exception  If a call was never made to run() or if the run was not incomplete (i.e. finished traversing the graph).
     */
    void resume();

    /*
     * @returns True if BFS traversal was completed, false if not (or if hasn't even been started).
     */
    bool isComplete() const;

	//graph to work on
	graph_ptr_t _graph;

private:
	//entries in BFS queue
	typedef struct
	{
		node_ptr_t	node;
		distance_t	distance;
	} bfs_entry_t;

    /*
     * @brief   Runs the BFS algorithm based on the queue's current situation.
     */
    void runBfsLoop();

    /*
     * @brief   This method is called for each node traversed during the BFS run. Nodes will (obviously) be called in BFS order.
	 * @param	curNode				The node the traversal is currently at.
	 * @param	prevNode			The node from which we arrived at this node.
	 * @param	distanceFromOrigin	The distance of curNode from the origin node.
     * @returns Dervied class implementation should return true to continue BFS iteration, false to stop (e.g. searched vertex was found).
     */
    virtual bool nodeTraversal(const node_ptr_t curNode, const node_ptr_t prevNode, distance_t distanceFromOrigin) = 0;

    //BFS queue
    std::queue<bfs_entry_t> _bfsQueue;

    //true if a run has been completed, false if stopped or unstarted
    bool _isCompleted;
};

} // namespace dhtoolkit

#endif //DELTAHYPER_BFS_H
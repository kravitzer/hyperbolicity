/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_SPANNING_TREE_H
#define DELTAHYPER_SPANNING_TREE_H

#include "BFS.h"
#include "defs.h"

namespace dhtoolkit
{
	/*
	 * @brief	This class generates a spanning tree on a given graph & starting node.
	 */
	class SpanningTree : public BFS
	{
	public:
		/*
		 * @brief	Creates a spanning tree on the graph, with the given root node.
		 * @param	graph	The graph to run on.
		 * @param	root	The root node of the tree.
		 */
		SpanningTree(const graph_ptr_t graph, const node_ptr_t root);

		/*
		 * @brief	Default dtor.
		 */
		virtual ~SpanningTree();

		/*
		 * @returns	The spanning tree calculated.
		 * @TODO: Currently user has access to inteernal graph... we could supply a copy but that would be inefficient. SOLVE!
		 */
		graph_ptr_t getTree() const;

	private:
		/*
		 * @brief	Derived class implementation of BFS - builds the spanning tree.
		 */
		bool nodeTraversal(const node_ptr_t curNode, const node_ptr_t prevNode, distance_t distanceFromOrigin);

		//holds the spanning tree calculated
		graph_ptr_t _tree;
	};
} // namespace dhtoolkit

#endif // DELTAHYPER_SPANNING_TREE_H
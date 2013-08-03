/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_STRONGLY_CONNECTED_COMPONENT_H
#define DELTAHYPER_STRONGLY_CONNECTED_COMPONENT_H

#include "BFS.h"
#include "defs.h"
#include "NodeHasher.h"
#include <memory>

namespace dhtoolkit
{
	/*
	 * @brief	This class iterates the graph from a given node and finds all the nodes in the same strongly-connected-component.
	 */
	class StronglyConnectedComponent : public BFS
	{
	public:
		/*
		 * @brief	Ctor receiving the graph and the node to start the iteration from.
		 */
		StronglyConnectedComponent(graph_ptr_t graph, node_ptr_t origin);

		/*
		 * @brief	Default dtor.
		 */
		virtual ~StronglyConnectedComponent();

		/*
		 * @returns	The nodes in the scc.
		 */
		const node_unordered_set_ptr_t getNodes() const;

	private:
		/*
		 * @brief	Derived class implementation - adds each node to the scc.
		 */
		bool nodeTraversal(const node_ptr_t curNode, const node_ptr_t prevNode, distance_t distanceFromOrigin);

		//stores the scc calculated
		node_unordered_set_ptr_t _scc;
	};

} // namespace dhtoolkit

#endif // DELTAHYPER_STRONGLY_CONNECTED_COMPONENT_H
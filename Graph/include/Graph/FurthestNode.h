/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#pragma once

#include "BFS.h"
#include "defs.h"

namespace hyperbolicity
{

	class FurthestNode : public BFS
	{
	public:
		/*
		 * @brief	Runs BFS on the given graph, from the given origin, and stores the results in member variables
		 *			accessible through accessor functions.
		 */
		FurthestNode(const graph_ptr_t graph, const node_ptr_t origin);

		/*
		 * @brief	Default virtual dtor.
		 */
		virtual ~FurthestNode();

		/*
		 * @returns	The collection of nodes that are furthest away from the origin.
		 */
		node_ptr_collection_t getFurthestNodes() const;

		/*
		 * @returns	The distance of the node(s) furthest away from the origin.
		 */
		distance_t getFurthestDistance() const;

	protected:
		/*
		 * @brief	Derived class implementation - keeps only the furthest nodes found.
		 */
		bool nodeTraversal(const node_ptr_t curNode, const node_ptr_t prevNode, distance_t distanceFromOrigin);

		//results
		node_ptr_collection_t _nodes;
		distance_t _distance;
	};

} // namespace hyperbolicity

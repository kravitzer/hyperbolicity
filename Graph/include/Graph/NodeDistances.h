/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#pragma once

#include "BFS.h"
#include "defs.h"

namespace hyperbolicity
{
	/*
	 * @brief	This class calculates the distance of every node from the given origin node.
	 */
	class NodeDistances : public BFS
	{
	public:
		/*
		 * @brief	Ctor receiving the graph to run on, and the node from which to calculate distances from.
		 */
		NodeDistances(const graph_ptr_t graph, const node_ptr_t origin);

		/*
		 * @brief	Default virtual dtor.
		 */
		virtual ~NodeDistances();
		
		/*
		 * @returns	A dictionary of the distances from the origin node
		 */
		distance_dict_t getDistances();

        /*
         * @returns The distance from the origin to the given destination node.
         */
        distance_t getDistance(node_ptr_t dest);

        /*
         * @returns The distance from the origin to the node whose index is given.
         */
        distance_t getDistance(node_index_t index);

        /*
         * @returns The distances from the origin to the given destination nodes.
         */
        distance_dict_t getDistances(const node_ptr_collection_t& dests);

	protected:
		/*
		 * @brief	Derived class implementation - keeps the shortest path to each node.
		 */
		bool nodeTraversal(const node_ptr_t curNode, const node_ptr_t prevNode, distance_t distanceFromOrigin);

		//distances are kept in this dictionary
		distance_dict_t _distances;

        //true if BFS traversal started, false initially
        bool _runStarted;

        //origin node to run from
        node_ptr_t _origin;

        //when this collection of nodes is found, the search stops. Empty means find all.
        node_ptr_collection_t _dests;

        //counts the number of nodes in _dests that have been found
        unsigned int _nodesFound;

    private:
        /*
         * @brief   Starts or continues the BFS traversal if necessary until all nodes in the node collection given are found.
         *          An empty collection means find all nodes.
         */
        void startTraversalForNodes(const node_ptr_collection_t& dests);
	};

}
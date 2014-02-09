/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */
 
 #pragma once
 
 #include "DeltaHyperbolicity.h"
 #include "Graph\defs.h"
#include <unordered_set>

namespace hyperbolicity
{
	/*
	 * @brief	A collection of some useful hyperbolicity-related methods in graphs.
	 */
	class HyperbolicityAlgorithms
	{
	public:
		struct DoubleSweepResult
		{
			node_ptr_t u;
			node_ptr_t v;
			distance_t dist;
			distance_dict_t uDistances;
		};
		
		/*
		 * @brief	Calculates the delta value of the given state nodes.
		 * @param	graph	The graph to calculate delta for.
		 * @param	state	The state for which we need to calculate the delta value.
		 * @returns	The calculated delta hyperbolicity value.
		 */
		static delta_t calculateDelta(const graph_ptr_t graph, const node_combination_t& state);

		/*
		 * @brief	Delta calculation considers all possible distances between 4 nodes (3 pairs of distances).
		 *			E.g. d1 = dist(v1, v2) + dist(v3, v4)
		 *				 d2 = dist(v1, v3) + dist(v2, v4)
		 *				 d3 = dist(v1, v4) + dist(v2, v3)
		 *			We then take the two largest numbers and the absolute value of their difference, over 2, is the delta hyperbolicity
		 *			for that node quad. This method calculates this value based on the 3 distance pairs.
		 * @returns	The delta obtained from the 3 distance pairs.
		 */
		static delta_t calculateDeltaFromDistances(distance_t d1, distance_t d2, distance_t d3);
		/*
		 * @brief	Runs a double-sweep on the given node. If several nodes are "the furthest", selects one randomly (equally distributed).
		 * @param	graph	The graph to run on.
		 * @param	origin	The node from which the double-sweep process starts. Optional. If not specified, the origin node is randomly selected.
		 * @returns	A DoubleSweepResult structure, holding the result of this process.
		 * @note	IMPORTANT! You must initialize a random seed (by calling srand() with some random seed) before calling this method, otherwise
		 *			you might get the same results each run!
		 */
		static DoubleSweepResult doubleSweep(const graph_ptr_t graph, const node_ptr_t origin = node_ptr_t(nullptr));

		/*
		 * @returns	A random state from the graph given.
		 */
		static node_combination_t getRandomState(const graph_ptr_t graph);

		/*
		 * @brief	Creates a Pajek net file that can be visualized using Pajek.
		 * @param	fileName		The name of the files to be created.
		 * @param	graph			The graph to be drawn.
		 * @param	nodesToMark		Nodes in this parameter will be given a unique color.
		 * @throws	std::exception	Upon any error.
		 */
		static void drawGraph(const std::string& fileName, const graph_ptr_t graph, const node_combination_t* nodesToMark);

		/*
		 * @brief	Removes the cycle that origin is a part of, if really is a part of a cycle, and if cycle may be removed without
		 *			affecting the hyperbolicity.
		 *
		 *			The cycle removal algorithm is as follows:
		 *				Starting at the origin node v,  for each of v's neighbors u: as long as degree(u) == 2, go to the neighbor that you did 
		 *				not come from. Of course, if we've somehow reached the first node - the graph is a simple cycle and we're done...
		 *				We now reached two vertices, s & t, whose degree > 2. Check d(s, t).
		 *				If d(s, t) < number of nodes iterated + 1 (excluding s & t) then the nodes iterated (excluding s & t) may be removed!
		 *				Before removal, we calculate the delta value produced from the cycle (truncate(length / 4) if length % 4 != 1, 
		 *				or truncate(length / 4) - 0.5 otherwise).
		 *				If d(s, t) = number of nodes iterated + 1 (excluding s & t), then unfortunatelly we cannot remove the cycle as it may
		 *				affect other distances in the graph. We simply calculate the delta of the cycle, assuming that the shortest path from s to
		 *				t that does not go through the chain of nodes iterated is of the same length (this yields a delta that cannot be larger than
		 *				the real value).
		 * @param	graph	The graph to run on.
		 * @param	origin	The node to start from.
		 * @param	delta	Will be set to the delta of the cycle found.
		 * @param	processedNodes		In case the cycle cannot be removed, the labels of the nodes processed will be added to this set (the user 
		 *								may keep them in order to avoid running on them in the future).
		 *								In case a cycle was removed, the labels of the nodes revmoed.
		 * @returns	Whether a cycle was removed from the graph or not.
		 */
		static bool removeCycle(graph_ptr_t graph, node_ptr_t origin, delta_t& delta, std::unordered_set<std::string>& processedNodes);

		/*
 		 * @brief	Prunes trees from the graph. I.e. removes all nodes with degree 0 or 1 (and those that become of this degree as a result
		 *			of these deletions, recursively).
		 * @param	graph	The graph to run on.
		 */
		static void pruneTrees(graph_ptr_t graph);

private:
		/*
		 * @param node	The node for which to perform the count.
		 * @returns The number of outgoing edges from the given node, pointing to unmarked nodes.
		 */
		static unsigned int countUnmarkedNeighbors(node_ptr_t node);

		/*
		 * @brief	Marks the given current leaf node and recursively calls itself on its neighbor (if exists).
		 * @param	curNode			The current node to mark for deletion. Must have degree 0 or 1.
		 * @returns The number of nodes marked for deletion.
		 * @throws	std::exception	If the current node's degree is greater than 1.
		 */
		static unsigned int pruneTreesRecursion(node_ptr_t curNode);

		/*
		 * @returns The first unmarked neighbor of the node given.
		 * @throws  std::exception if there is none.
		 */
		static node_ptr_t getUnmarkedNeighbor(const node_ptr_t node);

		/*
		 * @returns	The delta value of a cycle-graph whose length is given.
		 */
		static delta_t cycleDelta(size_t length);

		/*
		 * @brief Performs a single sweep on the graph - i.e. takes the origin node and randomly selects one of the nodes furthest away from it.
		 * @param	graph			The graph to run on.
		 * @param	origin			The node to perform the sweep from.
		 * @param	dist			Optional. Will be set to the distance of the node returned from the original node.
		 * @param	distancesFromU	Optional. Will be set to the distance collection from the origin node.
		 * @returns	The node selected from the sweep process.
		 */
		static node_ptr_t Sweep(const graph_ptr_t graph, const node_ptr_t origin, distance_t* dist, distance_dict_t* distancesFromU);

		/*
		 * @param	node			The node to be checked.
		 * @param	nodesToMark		The collection of nodes to be marked. May be null (function will return false).
		 * @returns	True if node is to be marked, false otherwise.
		 */
		static bool isNodeToBeMarked(node_ptr_t node, const node_combination_t* nodesToMark);
	};
} // namespace hyperbolicity
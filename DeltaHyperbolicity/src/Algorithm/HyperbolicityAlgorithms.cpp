#include "HyperbolicityAlgorithms.h"
#include "Graph/defs.h"
#include "Graph/Graph.h"
#include "Graph/Node.h"
#include "Graph/NodeDistances.h"
#include "Graph/FurthestNode.h"
#include "boost/format.hpp"

using namespace std;

namespace hyperbolicity
{
	delta_t HyperbolicityAlgorithms::calculateDelta(const graph_ptr_t graph, const node_combination_t& state)
	{
		node_ptr_collection_t nodeCollection;
		nodeCollection.push_back(state[1]);
		nodeCollection.push_back(state[2]);
		nodeCollection.push_back(state[3]);

		//calculate distances from v0 to v1, v2, v3
		NodeDistances NDFromS0(graph, state[0]);
		distance_dict_t v0dists = NDFromS0.getDistances(nodeCollection);

		//calculate distances from v1 to v2, v3
		nodeCollection.erase(nodeCollection.begin());
		NodeDistances NDFromS1(graph, state[1]);
		distance_dict_t v1dists = NDFromS1.getDistances(nodeCollection);

		//calculate distances from v2 to v3
		nodeCollection.erase(nodeCollection.begin());
		NodeDistances NDFromS2(graph, state[2]);
		distance_dict_t v2dists = NDFromS2.getDistances(nodeCollection);

		//d1 = dist(v0, v1) + dist(v2, v3)
		//d2 = dist(v0, v2) + dist(v1, v3)
		//d3 = dist(v0, v3) + dist(v1, v2)
		distance_t d1 = v0dists[state[1]->getIndex()] + v2dists[state[3]->getIndex()];
		distance_t d2 = v0dists[state[2]->getIndex()] + v1dists[state[3]->getIndex()];
		distance_t d3 = v0dists[state[3]->getIndex()] + v1dists[state[2]->getIndex()];

		return calculateDeltaFromDistances(d1, d2, d3);
	}

	delta_t HyperbolicityAlgorithms::calculateDeltaFromDistances(distance_t d1, distance_t d2, distance_t d3)
	{
		//find largest and second-largest distances out of the 3 options
		distance_t largest = d1, secondLargest = d1;
		if (d2 > largest)
		{
			largest = d2;
		}
		else
		{
			secondLargest = d2;
		}

		if (d3 > largest)
		{
			secondLargest = largest;
			largest = d3;
		}
		else
		{
			if (d3 > secondLargest) secondLargest = d3;
		}

		//return the delta value
		return static_cast<delta_t>(largest-secondLargest)/2;
	}

	HyperbolicityAlgorithms::DoubleSweepResult HyperbolicityAlgorithms::doubleSweep(const graph_ptr_t graph, const node_ptr_t origin /* = node_ptr_t(nullptr) */)
	{
		node_ptr_t startNode = origin;
		if (nullptr == startNode.get())
		{
			//randomly select start node
			startNode = graph->getNode(rand() % graph->size());
		}

		//perform the double sweep
		FurthestNode fn(graph, startNode);
		node_ptr_t firstSweepNode = fn.getFurthestNodes()[0];
		distance_t dist = 0;
		distance_dict_t distanceCollection;
		node_ptr_t secondSweepNode = Sweep(graph, firstSweepNode, &dist, &distanceCollection);

		//prepare the result structure and return it to caller
		HyperbolicityAlgorithms::DoubleSweepResult res = {0};
		res.u = firstSweepNode;
		res.v = secondSweepNode;
		res.dist = dist;
		res.uDistances = distanceCollection;

		return res;
	}

	node_combination_t HyperbolicityAlgorithms::getRandomState(const graph_ptr_t graph)
	{
		//choose initial state at random
		node_combination_t randomState;
		for (unsigned int i = 0; i < node_combination_t::size(); ++i)
		{
			bool isUnique = true;

			//repeat the following process:
			//select a random node, then make sure none of the previous nodes is the same as this one
			do
			{
                //select a random node
				node_index_t index = rand() % graph->size();
				randomState[i] = graph->getNode(index);

                //assume unique until proven otherwise
                isUnique = true;

				//compare to all previous nodes
				for (unsigned int j = 0; (j < i) && isUnique; ++j)
				{
					if (randomState[j]->getIndex() == index) isUnique = false;
				}
			} while (!isUnique);

			//node at place i has been selected, move on to next index
		}

		return randomState;
	}

	void HyperbolicityAlgorithms::drawGraph(const string& fileName, const graph_ptr_t graph, const node_combination_t* nodesToMark)
	{
		shared_ptr<FILE> file(_fsopen(fileName.c_str(), "wb", _SH_DENYRW), &fclose);
		if (nullptr == file.get())
		{
			throw exception("Failed creating Pajek file");
		}

		string vertices;
		vertices += (boost::format("*Vertices %1%\n") % graph->size()).str();

		string edges = "*EdgesList\n";

		for (unsigned int i = 0; i < graph->size(); ++i)
		{
			node_ptr_t curNode = graph->getNode(i);
			node_weak_ptr_collection_t neighbors = curNode->getEdges();

			if (isNodeToBeMarked(curNode, nodesToMark))
			{
				vertices += (boost::format("%1% \"%2%\" ic Blue\n") % (i+1) % i).str();
			}
			else
			{
				vertices += (boost::format("%1% \"%2%\" ic Red\n") % (i+1) % i).str();
			}

			edges += (boost::format("%1%") % (i+1)).str();
			for (node_weak_ptr_collection_t::const_iterator it = neighbors.cbegin(); it != neighbors.cend(); ++it)
			{
				edges += (boost::format(" %1%") % (it->lock()->getIndex() + 1)).str();
			}
			edges += "\n";
		}

		if (vertices.size() != fwrite(vertices.c_str(), sizeof(char), vertices.size(), file.get()))
		{
			throw exception("Error writing graph nodes to file");
		}
		if (edges.size() != fwrite(edges.c_str(), sizeof(char), edges.size(), file.get()))
		{
			throw exception("Error writing graph edges to file");
		}
	}

	bool HyperbolicityAlgorithms::removeCycle(graph_ptr_t graph, node_ptr_t origin, delta_t& delta, unordered_set<string>& processedNodes)
	{
		//the chains of nodes from each side of origin to be deleted, if possible (excluding origin)
		node_ptr_collection_t chain1, chain2;

		node_weak_ptr_collection_t neighbors = origin->getEdges();
		if (neighbors.size() != 2) return 0;

		node_weak_ptr_t n1 = neighbors[0];
		node_weak_ptr_t n2 = neighbors[1];

		//the nodes that end the cycle
		node_ptr_t s, t;

		//start walking on chain1
		node_weak_ptr_t prev = origin;
		node_weak_ptr_t cur1 = neighbors[0];
		node_weak_ptr_collection_t curNeighbors = cur1.lock()->getEdges();
		for (; curNeighbors.size() == 2; curNeighbors = cur1.lock()->getEdges())
		{
			chain1.push_back(cur1.lock());

			node_weak_ptr_t tmp = cur1;
			cur1 = (curNeighbors[0].lock() == prev.lock() ? curNeighbors[1] : curNeighbors[0]);
			prev = tmp;

			//check extreme case: graph is a cycle?
			if (cur1.lock() == neighbors[1].lock())
			{
				//graph is a single cycle!
				delta_t curDelta = cycleDelta(graph->size());
				graph.reset(new Graph(graph->getTitle()));
				delta = curDelta;
				return true;
			}
		}

		//start walking on chain2
		prev = origin;
		node_weak_ptr_t cur2 = neighbors[1];
		curNeighbors = cur2.lock()->getEdges();
		for (; curNeighbors.size() == 2; curNeighbors = cur2.lock()->getEdges())
		{
			chain2.push_back(cur2.lock());

			node_weak_ptr_t tmp = cur2;
			cur2 = (curNeighbors[0].lock() == prev.lock() ? curNeighbors[1] : curNeighbors[0]);
			prev = tmp;
		}

		NodeDistances distCalculator(graph, cur1.lock());
		distance_t stDist = distCalculator.getDistance(cur2.lock());

		//add this chain's nodes to the irremovable set
		processedNodes.insert(origin->getLabel());
		for (auto it = chain1.cbegin(); it != chain1.cend(); ++it)
		{
			processedNodes.insert((*it)->getLabel());
		}
		for (auto it = chain2.cbegin(); it != chain2.cend(); ++it)
		{
			processedNodes.insert((*it)->getLabel());
		}

		if (stDist == chain1.size() + chain2.size() + 2)
		{
			//we cannot remove the cycle as it may affect the value of delta

			//calculate maximal cycle size
			size_t cycleSize = (chain1.size() + chain2.size() + 2) * 2;
			delta = cycleDelta(cycleSize);
			return false;
		}
		else if (static_cast<size_t>(stDist) < chain1.size() + chain2.size() + 2)
		{
			//calculate cycle size
			size_t cycleSize = chain1.size() + chain2.size() + 2 + stDist;

			//remove cycle!
			graph->unmarkNodes();
			origin->setMarked(true);
			for (auto it = chain1.begin(); it != chain1.cend(); ++it) (*it)->setMarked(true);
			for (auto it = chain2.begin(); it != chain2.cend(); ++it) (*it)->setMarked(true);
			graph->deleteMarkedNodes();

			delta = cycleDelta(cycleSize);
			return true;
		}
		else
		{
			//impossible - distance from s to t is at most the chains' length + 1 (origin)
			throw exception("Impossible distance received");
		}
	}

	void HyperbolicityAlgorithms::pruneTrees(graph_ptr_t graph)
    {
        //unmark all nodes
        graph->unmarkNodes();

        unsigned int removed = 0;

        //run on all nodes, mark the ones that need to be removed
        for (unsigned int i = 0; i < graph->size(); ++i)
        {
			node_ptr_t curNode = graph->getNode(i);
            //check if current node needs to be pruned & not already pruned
            if ( (countUnmarkedNeighbors(curNode) <= 1) && (!curNode->isMarked()) )
            {
                //prune node recursively
                removed += pruneTreesRecursion(curNode);
            }
        }

        //instead of deleting a ton of unnecessary nodes (deletion is VERY expensive!), we create
        //a new collection and copy only the necessary nodes into it, while setting their index appropriately
        //tests: using deletion, pruning of a large graph (1M+ nodes) took ~310 seconds. with this method - 0.238 seconds!!!

        graph->deleteMarkedNodes();
    }

    unsigned int HyperbolicityAlgorithms::countUnmarkedNeighbors(const node_ptr_t node)
    {
        unsigned int unmarkedNodes = 0;
        const node_weak_ptr_collection_t& neighbors = node->getEdges();
        for (node_weak_ptr_collection_t::const_iterator it = neighbors.cbegin(); it != neighbors.cend(); ++it)
        {
            if (!it->lock()->isMarked()) ++unmarkedNodes;
        }

        return unmarkedNodes;
    }

	unsigned int HyperbolicityAlgorithms::pruneTreesRecursion(node_ptr_t curNode)
    {
        unsigned int edgesLen = countUnmarkedNeighbors(curNode);
        
        //assert degree is <= 1
        if (edgesLen > 1) throw std::exception("Pruned node degree is > 1");

        //if node has a neighbor (can have only 1, if any), remember it before we remove the edge
        node_ptr_t neighbor(nullptr);
        if (1 == edgesLen) neighbor = getUnmarkedNeighbor(curNode);

        //mark node for pruning
        curNode->setMarked(true);
        unsigned int marked = 1;

        //if the pruned node had a neighbor, and it has now become a leaf, prune it recursively
        if ( (nullptr != neighbor.get()) && (countUnmarkedNeighbors(neighbor)) <= 1) marked += pruneTreesRecursion(neighbor);

        return marked;
    }

    node_ptr_t HyperbolicityAlgorithms::getUnmarkedNeighbor(const node_ptr_t node)
    {
        const node_weak_ptr_collection_t& neighbors = node->getEdges();
        for (node_weak_ptr_collection_t::const_iterator it = neighbors.cbegin(); it != neighbors.cend(); ++it)
        {
            if (!it->lock()->isMarked()) return it->lock();
        }

        throw std::exception("No unmarked neighbor found!");
    }

    delta_t HyperbolicityAlgorithms::cycleDelta(size_t length)
	{
		unsigned int remainder = (length % 4);
		size_t p = length / 4;
		if (1 == remainder) return p-0.5;
		else return static_cast<delta_t>(p);
	}

	node_ptr_t HyperbolicityAlgorithms::Sweep(const graph_ptr_t graph, const node_ptr_t origin, distance_t* dist, distance_dict_t* distancesFromU)
	{
		NodeDistances NDFromOrigin(graph, origin);
		distance_dict_t startNodeDistances = NDFromOrigin.getDistances();
		if (distancesFromU) *distancesFromU = startNodeDistances;

		//find nodes at maximal distance
        node_ptr_collection_t furthestNodes;
		distance_t maxDistance = 0;
		for (distance_dict_t::const_iterator it = startNodeDistances.cbegin(); it != startNodeDistances.cend(); ++it)
		{
			if (InfiniteDistance == it->second) continue;
			if (it->second >= maxDistance)
            {
                if (it->second > maxDistance)
                {
                    furthestNodes.clear();
                    maxDistance = it->second;
                }
                furthestNodes.push_back(graph->getNode(it->first));
            }
		}

		//randomly select node out of the furthest nodes
		unsigned int selectedNodeIndex = rand() % furthestNodes.size();
		if (dist) *dist = maxDistance;
		return furthestNodes[selectedNodeIndex];
	}

    bool HyperbolicityAlgorithms::isNodeToBeMarked(node_ptr_t node, const node_combination_t* nodesToMark)
	{
		if (nullptr == nodesToMark) return false;

		for (unsigned int i = 0; i < nodesToMark->size(); ++i)
		{
			if (nodesToMark->operator[](i) == node) return true;
		}

		return false;
	}

} // namespace hyperbolicity
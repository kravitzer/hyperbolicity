#include "GraphBuilder.h"
#include <random>
#include <time.h>
#include <unordered_set>
#include <vector>
#include <Windows.h>
#include "Graph.h"
#include "Node.h"
#include "GraphAlgorithms.h"

using namespace std;

namespace hyperbolicity
{

minstd_rand randEngine(static_cast<unsigned int>(time(NULL)) + GetCurrentProcessId());

graph_ptr_t GraphBuilder::createERGraph(unsigned int nodeCount, double p)
{
	graph_ptr_t g(new Graph("ER_Graph"));
	for (unsigned int i = 0; i < nodeCount; ++i)
	{
		g->insertNode();
	}

	//calculate edges

	//on VS2012 the uniform_int_distribution runs *much* slower than on VS2010...
	//to resolve this we use uniform_real_distribution instead.
	//see: http://social.msdn.microsoft.com/Forums/en-US/vcgeneral/thread/cff484b1-781a-4e9f-935c-7c2d0c3a4ffe
	uniform_real_distribution<> d(0, 999999);
	for (unsigned int i = 0; i < nodeCount; ++i)
	{
		for (unsigned int j = i+1; j < nodeCount; ++j)
		{
			int randNum = static_cast<int>(d(randEngine));
			if (randNum < p * 1000000)
			{
				g->getNode(i)->insertBidirectionalEdgeTo(g->getNode(j));
			}
		}
	}

	return g;
}


unsigned int GraphBuilder::calculateNextM(double m)
{
	//returns ceiling(m) with probability m-trunc(m) or floor(m) the rest of the time.

	unsigned int cur_m = static_cast<int>(m);
	unsigned int addProb = static_cast<unsigned int>(static_cast<float>(m - static_cast<int>(m))*10000);
	uniform_real_distribution<> randomGen(1, 10000);
	unsigned int r = static_cast<unsigned int>(randomGen(randEngine));
	if (r <= addProb) ++cur_m;

	return cur_m;
}

graph_ptr_t GraphBuilder::createBAGraph(unsigned int n, double m)
{
	graph_ptr_t g(new Graph("mygraph"));
	unordered_set<node_index_t> targets;
	vector<node_index_t> repeatedNodes;

	//since m may not be an integer, we will define for each round a current m - which will be either ceiling(m)
	//with probability m-floor(m) or floor(m) the rest of the time.
	unsigned int cur_m = calculateNextM(m);

	//add first m nodes (no edges yet)
	for (node_index_t i = 0; i < cur_m; ++i)
	{
		g->insertNode();
		targets.emplace(i);
	}

	//add the rest of the nodes, with the required number of edges per each created node
	for (node_index_t source = cur_m; source < n; ++source)
	{
		//create new node and connect it to the target nodes
		node_ptr_t newNode = g->insertNode();
		for (auto it = targets.cbegin(); it != targets.cend(); ++it)
		{
			newNode->insertBidirectionalEdgeTo(g->getNode(*it));
			repeatedNodes.push_back(*it);
			repeatedNodes.push_back(newNode->getIndex());
		}

		cur_m = calculateNextM(m);
		targets.clear();
		uniform_int_distribution<> d(0, static_cast<int>(repeatedNodes.size()-1));
		while (targets.size() < cur_m)
		{
			int r = d(randEngine);
			targets.emplace(repeatedNodes[r]);
		}
	}

	return g;
}

graph_ptr_t GraphBuilder::createBAExtendedGraph(unsigned int n, double m, double p)
{
	graph_ptr_t g(new Graph("mygraph"));
	unordered_set<node_index_t> targets;
	vector<node_index_t> repeatedNodes;

	//since m may not be an integer, we will define for each round a current m - which will be either ceiling(m)
	//with probability m-floor(m) or floor(m) the rest of the time.
	unsigned int cur_m = calculateNextM(m);

	//add first m nodes (no edges yet)
	for (node_index_t i = 0; i < cur_m; ++i)
	{
		g->insertNode();
		targets.emplace(i);
	}

	unsigned int step = 0;
	while (g->size() < n)
	{
		++step;
		unsigned int addProb = static_cast<unsigned int>(p*10000);
		uniform_int_distribution<> randomGen(1, 10000);
		unsigned int r = randomGen(randEngine);

		if (step < 15) r = 10000;

		if (r <= addProb)
		{
			auto it = targets.cbegin();
			for (unsigned int i = 0; i < cur_m; ++i, ++it)
			{
				if (targets.cend() == it) throw exception("targets collection too small!");

				uniform_int_distribution<> randomGen2(0, static_cast<int>(g->size()-1));
				node_ptr_t randNode = g->getNode(randomGen2(randEngine));
				
				//make sure the edge to be added doesn't already exist and that it is not a self-edge
				if ( (*it == randNode->getIndex()) || (randNode->hasEdge(g->getNode(*it)) ) ) continue;
				randNode->insertBidirectionalEdgeTo(g->getNode(*it));
				repeatedNodes.push_back(*it);
				repeatedNodes.push_back(randNode->getIndex());
			}
		}
		else
		{
			//create new node and connect it to the target nodes
			node_ptr_t newNode = g->insertNode();
			for (auto it = targets.cbegin(); it != targets.cend(); ++it)
			{
				newNode->insertBidirectionalEdgeTo(g->getNode(*it));
				repeatedNodes.push_back(*it);
				repeatedNodes.push_back(newNode->getIndex());
			}
		}

		//calculate parameters for next iteration
		cur_m = calculateNextM(m);
		//this solves a bug where the first step is to add links (without nodes), however each new link to be added is a self-edge
		//and therefore no edges are added. repeatedNodes remain at size 0. If this happens we simply skip to the next step, keeping
		//targets as they are, hoping that on the next step (or the ones after that) we will eventually either get a link between
		//the 2 nodes or get a 3rd node that would be connected to the 2 nodes.
		if (repeatedNodes.size() == 0) continue;
		uniform_int_distribution<> d(0, static_cast<int>(repeatedNodes.size()-1));
		targets.clear();
		while (targets.size() < cur_m)
		{
			int r = d(randEngine);
			targets.emplace(repeatedNodes[r]);
		}
	}

	return g;
}

} //namespace hyperbolicity
#include "SimulatedAnnealing.h"
#include "SADefs.h"
#include "Graph.h"
#include "GraphAlgorithms.h"
#include <random>
#include <string>

using namespace std;

namespace dhtoolkit
{

	SimulatedAnnealing::SimulatedAnnealing(const string& outputDir, sa_prob_func_ptr probabilityFunction, sa_temp_func_ptr tempFunction, sa_callback_func_ptr callback) : IGraphAlg(outputDir),
		_temp(0), _probFunc(probabilityFunction), _tempFunc(tempFunction), _callbackFunc(callback)
	{
		//empty
	}

	SimulatedAnnealing::~SimulatedAnnealing()
	{
		//empty
	}

	DeltaHyperbolicity SimulatedAnnealing::runImpl(const graph_ptr_t graph)
	{
		//note - we assume it's already been checked that the input is legal - i.e. valid pointer to a graph of at least node_quad_t::size() nodes

		//choose initial state at random
		node_quad_t randomState;
		for (unsigned int i = 0; i < node_quad_t::size(); ++i)
		{
			bool isUnique = true;

			//repeat the following process:
			//select a random node, then make sure none of the previous nodes is the same as this one
			do
			{
				//select a random node
				node_index_t index = rand() % graph->size();
				randomState[i] = graph->getNode(index);

				//compare to all previous nodes
				for (unsigned int j = 0; (j < i) && isUnique; ++j)
				{
					if (randomState[j]->getIndex() == index) isUnique = false;
				}
			} while (!isUnique);

			//node at place i has been selected, move on to next index
		}

		return startSA(graph, randomState);
	}

	DeltaHyperbolicity SimulatedAnnealing::runWithInitialStateImpl(const graph_ptr_t graph, const node_quad_t& initialState)
	{
		return startSA(graph, initialState);
	}

	void SimulatedAnnealing::reset()
	{
		_probFunc->reset();
		_tempFunc->reset();
		_callbackFunc->reset();
		_temp = _tempFunc->GetInitialTemperature();
	}

	DeltaHyperbolicity SimulatedAnnealing::startSA(const graph_ptr_t graph, const node_quad_t& initialState)
	{
		node_quad_t curState, maxState;
		delta_t curDelta, maxDelta;

		//copy initial state as current state
		maxState = curState = initialState;
		maxDelta = curDelta = GraphAlgorithms::CalculateDelta(graph, curState);

		//TODO: optimization - for each current state we can hold the Dijksra's of each of its nodes. Then when performing a step to
		//a neighboring state, we change only one node, so we may perform Dijkstra on that single node only (as opposed to 3 different
		//Dijkstra's) to calculate the new delta. Perhaps we could think of a way to "modify" the replaced node's Dijkstra such that
		//calculating its neighbor's Dijkstra would take only ~O(E[i]) time instead of O(|E|+|V|log|V|)!
		
		//loop as long as temperature is positive
		while (_temp > 0)
		{
			//call callback method if exists
			if (nullptr != _callbackFunc.get()) _callbackFunc->callback(graph, curState, curDelta, _temp, false);

			//perform a single step
			node_quad_t newState;
			step(graph, curState, &newState);

			//calculate new step's new delta
			delta_t newDelta = GraphAlgorithms::CalculateDelta(graph, newState);

			//if it is a current max, update the max values
			if (newDelta > maxDelta)
			{
				maxDelta = newDelta;
				maxState = newState;
			}

			//run the probability method to see if we accept the new state
			sa_probability_t prob = _probFunc->ProbabilityToAcceptChange(curDelta, newDelta, _temp);
			bool accept = ( (static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) <= prob );

			//update the temperature
			_temp = _tempFunc->TemperatureChange(_temp, curDelta, newDelta);

			//update current state if it was accepted
			if (accept)
			{
				curState = newState;
				curDelta = newDelta;
			}
		}

		if (nullptr != _callbackFunc.get()) _callbackFunc->callback(graph, maxState, maxDelta, _temp, true);

		//return the maximal delta found throughout the process
		return DeltaHyperbolicity(maxDelta, maxState);
	}

	void SimulatedAnnealing::step(const graph_ptr_t graph, const node_quad_t& curState, node_quad_t* newState) const
	{
		bool isUniqueNeighborFound = false;
		unsigned int replacedNodeIndex = 0;
		node_ptr_t newNode;

		//repeat the process as long as we haven't found a new unique node (i.e. for a given state, two nodes
		//may be neighbors, and we might (by chance) select a node that is already in the state thus receiving
		//a new state with nodes that are not unique... so we repeat the random selection until we receive a state
		//where each node is in fact unique)
		while (!isUniqueNeighborFound)
		{
			//select a random node to replace
			replacedNodeIndex = rand() % node_quad_t::size();
			node_ptr_t nodeReplaced = curState[replacedNodeIndex];

			//select a neighbor node randomly
			const node_collection_t& neighbors = nodeReplaced->getEdges();
			unsigned int neighborIndex = rand() % neighbors.size();
			newNode = neighbors[neighborIndex];

			//assume new node is unique (i.e. not already in current state) and go through current state
			//to see if this assumption is wrong...
			isUniqueNeighborFound = true;
			for (unsigned int i = 0; (i < node_quad_t::size()) && (isUniqueNeighborFound); ++i)
			{
				//if new node is already in the current state, it is not unique!
				if (curState[i]->getIndex() == newNode->getIndex()) isUniqueNeighborFound = false;
			}
		}

		//modify new state data type, replacing the old node with the new one
		for (unsigned int i = 0; i < node_quad_t::size(); ++i)
		{
			if (i != replacedNodeIndex) 
			{
				(*newState)[i] = curState[i];
			}
			else
			{
				(*newState)[i] = newNode;
			}
		}
	}
} // namespace dhtoolkit
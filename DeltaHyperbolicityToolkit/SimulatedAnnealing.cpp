#include "SimulatedAnnealing.h"
#include "SADefs.h"
#include "Graph.h"
#include "GraphAlgorithms.h"
#include "NodeDistances.h"
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

        //distances are ordered as follows in the following array: d_01, d_02, d_03, d_12, d_13, d_23
        distance_t nodeDistances[6];
        //used to mark the nodes we're looking for in every search
        node_collection_t destinationNodes;

		//scope in order to release NodeDistances' memory after finding distances
		{
			NodeDistances NDFrom0(graph, curState[0]);
			NodeDistances NDFrom1(graph, curState[1]);
			NodeDistances NDFrom2(graph, curState[2]);
			//calculate d_0X (d_01, d_02, d_3)
			destinationNodes.clear();
			destinationNodes.push_back(curState[1]);
			destinationNodes.push_back(curState[2]);
			destinationNodes.push_back(curState[3]);
			distance_dict_t distances = NDFrom0.getDistances(destinationNodes);
			nodeDistances[0] = distances[curState[1]->getIndex()];
			nodeDistances[1] = distances[curState[2]->getIndex()];
			nodeDistances[2] = distances[curState[3]->getIndex()];
			//calculate d_1X (d1_2, d_13)
			destinationNodes.clear();
			destinationNodes.push_back(curState[2]);
			destinationNodes.push_back(curState[3]);
			distances = NDFrom1.getDistances(destinationNodes);
			nodeDistances[3] = distances[curState[2]->getIndex()];
			nodeDistances[4] = distances[curState[3]->getIndex()];
			//calculate d_2X (d_23)
			destinationNodes.clear();
			destinationNodes.push_back(curState[3]);
			distances = NDFrom2.getDistances(destinationNodes);
			nodeDistances[5] = distances[curState[3]->getIndex()];
		}

        //calculate current delta
        distance_t d1 = nodeDistances[0] + nodeDistances[5];
        distance_t d2 = nodeDistances[1] + nodeDistances[4];
        distance_t d3 = nodeDistances[2] + nodeDistances[3];
		maxDelta = curDelta = GraphAlgorithms::CalculateDeltaFromDistances(d1, d2, d3);

		//loop as long as temperature is positive
		while (_temp > 0)
		{
			//call callback method if exists
			if (nullptr != _callbackFunc.get()) _callbackFunc->callback(graph, curState, curDelta, _temp, false);

			//perform a single step
			node_quad_t newState;
			unsigned int replacedNodeIndexInState = step(graph, curState, &newState);

            //need to calculate distances from new node to other 3
            destinationNodes.clear();
            for (int i = 0; i < 4; ++i)
            {
                if (replacedNodeIndexInState != i) destinationNodes.push_back(newState[i]);
            }
			NodeDistances NDFromReplacedNode(graph, newState[replacedNodeIndexInState]);
			distance_dict_t distances = NDFromReplacedNode.getDistances(destinationNodes);

            //reset new distances to a side array
            distance_t newNodeDistances[6];
            copy(nodeDistances, nodeDistances+6, newNodeDistances);
            //start from current array, replace distances that have changed
            switch (replacedNodeIndexInState)
            {
            case 0:
                newNodeDistances[0] = distances[newState[1]->getIndex()];
                newNodeDistances[1] = distances[newState[2]->getIndex()];
                newNodeDistances[2] = distances[newState[3]->getIndex()];
                break;

            case 1:
                newNodeDistances[0] = distances[newState[0]->getIndex()];
                newNodeDistances[3] = distances[newState[2]->getIndex()];
                newNodeDistances[4] = distances[newState[3]->getIndex()];
                break;

            case 2:
                newNodeDistances[1] = distances[newState[0]->getIndex()];
                newNodeDistances[3] = distances[newState[1]->getIndex()];
                newNodeDistances[5] = distances[newState[3]->getIndex()];
                break;

            case 3:
                newNodeDistances[2] = distances[newState[0]->getIndex()];
                newNodeDistances[4] = distances[newState[1]->getIndex()];
                newNodeDistances[5] = distances[newState[2]->getIndex()];
                break;

            default:
                throw exception("Invalid index for replaced node");
            }

            //recalculate distances of perfect matchings
            d1 = newNodeDistances[0] + newNodeDistances[5];
            d2 = newNodeDistances[1] + newNodeDistances[4];
            d3 = newNodeDistances[2] + newNodeDistances[3];

            //calculate new step's new delta
			delta_t newDelta = GraphAlgorithms::CalculateDeltaFromDistances(d1, d2, d3);

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
                copy(newNodeDistances, newNodeDistances+6, nodeDistances);
			}
		}

		if (nullptr != _callbackFunc.get()) _callbackFunc->callback(graph, maxState, maxDelta, _temp, true);

		//return the maximal delta found throughout the process
		return DeltaHyperbolicity(maxDelta, maxState);
	}

	unsigned int SimulatedAnnealing::step(const graph_ptr_t graph, const node_quad_t& curState, node_quad_t* newState) const
	{
		bool isUniqueNeighborFound = false;
		unsigned int replacedNodeIndexInState = 0;
		node_ptr_t newNode = nullptr;

		//repeat the process as long as we haven't found a new unique node (i.e. for a given state, two nodes
		//may be neighbors, and we might (by chance) select a node that is already in the state thus receiving
		//a new state with nodes that are not unique... so we repeat the random selection until we receive a state
		//where each node is in fact unique)
		while (!isUniqueNeighborFound)
		{
			//select a random node to replace
			replacedNodeIndexInState = rand() % node_quad_t::size();
			node_ptr_t nodeReplaced = curState[replacedNodeIndexInState];
            
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
			if (i != replacedNodeIndexInState) 
			{
				(*newState)[i] = curState[i];
			}
			else
			{
				(*newState)[i] = newNode;
			}
		}

        return replacedNodeIndexInState;
	}
} // namespace dhtoolkit
#include "SimulatedAnnealing.h"
#include "SADefs.h"
#include "Graph\Graph.h"
#include "Graph\GraphAlgorithms.h"
#include "Graph\NodeDistances.h"
#include "Algorithm\HyperbolicityAlgorithms.h"
#include <random>
#include <string>

using namespace std;

namespace dhtoolkit
{

	SimulatedAnnealing::SimulatedAnnealing(sa_prob_func_ptr probabilityFunction, sa_temp_func_ptr tempFunction, sa_callback_func_ptr callback) : IGraphAlg(),
										_temp(0), 
										_probFunc(probabilityFunction), 
										_tempFunc(tempFunction), 
										_callbackFunc(callback),
										_curState(),
										_curDelta(0)
	{
		//empty
	}

	SimulatedAnnealing::~SimulatedAnnealing()
	{
		//empty
	}

	bool SimulatedAnnealing::isComplete() const
	{
		return _temp <= 0;
	}

	void SimulatedAnnealing::initImpl(const node_combination_t& initialState)
	{
		//reset SA parameters
		_probFunc->reset();
		_tempFunc->reset();
		_callbackFunc->reset();
		_temp = _tempFunc->GetInitialTemperature();
		_isFirstStep = true;

		//set current state to state given, or a random state if no state is given
		_curState = ( initialState.isInitialized() ? initialState : HyperbolicityAlgorithms::getRandomState(_graph) );
	}

	delta_t SimulatedAnnealing::calculateCurrentDelta()
	{
		//calculate node distances for current state
		NodeDistances NDFrom0(_graph, _curState[0]);
		NodeDistances NDFrom1(_graph, _curState[1]);
		NodeDistances NDFrom2(_graph, _curState[2]);
		//calculate d_0X (d_01, d_02, d_3)
		_destinationNodes.clear();
		_destinationNodes.push_back(_curState[1]);
		_destinationNodes.push_back(_curState[2]);
		_destinationNodes.push_back(_curState[3]);
		distance_dict_t distances = NDFrom0.getDistances(_destinationNodes);
		_nodeDistances[0] = distances[_curState[1]->getIndex()];
		_nodeDistances[1] = distances[_curState[2]->getIndex()];
		_nodeDistances[2] = distances[_curState[3]->getIndex()];
		//calculate d_1X (d1_2, d_13)
		_destinationNodes.clear();
		_destinationNodes.push_back(_curState[2]);
		_destinationNodes.push_back(_curState[3]);
		distances = NDFrom1.getDistances(_destinationNodes);
		_nodeDistances[3] = distances[_curState[2]->getIndex()];
		_nodeDistances[4] = distances[_curState[3]->getIndex()];
		//calculate d_2X (d_23)
		_destinationNodes.clear();
		_destinationNodes.push_back(_curState[3]);
		distances = NDFrom2.getDistances(_destinationNodes);
		_nodeDistances[5] = distances[_curState[3]->getIndex()];

		//calculate current delta
        distance_t d1 = _nodeDistances[0] + _nodeDistances[5];
        distance_t d2 = _nodeDistances[1] + _nodeDistances[4];
        distance_t d3 = _nodeDistances[2] + _nodeDistances[3];
		return HyperbolicityAlgorithms::calculateDeltaFromDistances(d1, d2, d3);
	}

	DeltaHyperbolicity SimulatedAnnealing::stepImpl()
	{
		//for first step, simply calculate its delta and return the state
		if (_isFirstStep)
		{
			_curDelta = calculateCurrentDelta();
			_isFirstStep = false;
			return DeltaHyperbolicity(_curDelta, _curState);
		}

		//call callback method if exists
		if (nullptr != _callbackFunc.get()) _callbackFunc->callback(_graph, _curState, _curDelta, _temp, false);

		//perform a single step
		node_combination_t newState;
		unsigned int replacedNodeIndexInState = getNeighbor(_graph, _curState, &newState);

        //need to calculate distances from new node to other 3
        _destinationNodes.clear();
        for (int i = 0; i < 4; ++i)
        {
            if (replacedNodeIndexInState != i) _destinationNodes.push_back(newState[i]);
        }
		NodeDistances NDFromReplacedNode(_graph, newState[replacedNodeIndexInState]);
		distance_dict_t distances = NDFromReplacedNode.getDistances(_destinationNodes);

        //reset new distances to a side array
        distance_t newNodeDistances[6];
        copy(_nodeDistances, _nodeDistances+6, newNodeDistances);
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
        distance_t d1 = newNodeDistances[0] + newNodeDistances[5];
        distance_t d2 = newNodeDistances[1] + newNodeDistances[4];
        distance_t d3 = newNodeDistances[2] + newNodeDistances[3];

        //calculate new step's new delta
		delta_t newDelta = HyperbolicityAlgorithms::calculateDeltaFromDistances(d1, d2, d3);

		//run the probability method to see if we accept the new state
		sa_probability_t prob = _probFunc->ProbabilityToAcceptChange(_curDelta, newDelta, _temp);
		bool accept = ( (static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) <= prob );

		//update the temperature
		_temp = _tempFunc->TemperatureChange(_temp, _curDelta, newDelta);

		//update current state if it was accepted
		if (accept)
		{
			_curState = newState;
			_curDelta = newDelta;
            copy(newNodeDistances, newNodeDistances+6, _nodeDistances);
		}

		return DeltaHyperbolicity(_curDelta, _curState);
	}

	unsigned int SimulatedAnnealing::getNeighbor(const graph_ptr_t graph, const node_combination_t& curState, node_combination_t* newState) const
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
			replacedNodeIndexInState = rand() % node_combination_t::size();
			node_ptr_t nodeReplaced = curState[replacedNodeIndexInState];
            
			//select a neighbor node randomly
			const node_weak_ptr_collection_t& neighbors = nodeReplaced->getEdges();
			unsigned int neighborIndex = rand() % neighbors.size();
			newNode = neighbors[neighborIndex].lock();

			//assume new node is unique (i.e. not already in current state) and go through current state
			//to see if this assumption is wrong...
			isUniqueNeighborFound = true;
			for (unsigned int i = 0; (i < node_combination_t::size()) && (isUniqueNeighborFound); ++i)
			{
				//if new node is already in the current state, it is not unique!
				if (curState[i]->getIndex() == newNode->getIndex()) isUniqueNeighborFound = false;
			}
		}

		//modify new state data type, replacing the old node with the new one
		for (unsigned int i = 0; i < node_combination_t::size(); ++i)
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
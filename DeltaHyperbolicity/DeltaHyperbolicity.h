#ifndef DELTAHYPER_DELTA_HYPERBOLICITY_H
#define DELTAHYPER_DELTA_HYPERBOLICITY_H

#include "defs.h"

namespace graphs
{

/*
 * @brief	Represents a delta hyperbolicity result.
 */
class DeltaHyperbolicity
{
public:
	/*
	 * @param	delta	The delta hyperbolicity value.
	 */
	DeltaHyperbolicity(delta_t delta) : _delta(delta), _nodesAvailable(false), _nodes() 
	{
		//empty
	}

	/*
	 * @param	delta	The delta hyperbolicity value.
	 * @param	nodes	The nodes that convey the delta value.
	 * @note	No check is made to make sure the nodes given in face produce the given delta value.
	 */
	DeltaHyperbolicity(delta_t delta, node_quad_t nodes) : _delta(delta), _nodesAvailable(true)
	{
		for (unsigned int i = 0; i < NodeQuadCount; ++i)
		{
			_nodes[i] = nodes[i];
		}
	}

	/*
	 * @returns	The delta value represented by this instance.
	 */
	delta_t getDelta() const
	{
		return _delta;
	}

	/*
	 * @returns	The nodes that produce this instance's delta value.
	 * @throws	DataUnavailableException	If this instance was created without specifying these nodes.
	 */
	const node_quad_t& getNodes() const
	{
		return _nodes;
	}

	//allow default copy ctor / assignment operator
	//DeltaHyperbolicity(const DeltaHyperbolicity&);
	//DeltaHyperbolicity& operator=(const DeltaHyperbolicity&);

private:
	//the delta hyperbolicity value
	delta_t _delta;
	//the nodes that give the current delta hyperbolicity value
	node_quad_t _nodes;
	//were the nodes specified upon construction? if not, this value will be false, and an exception
	//will be thrown if the user tries to access these nodes.
	bool _nodesAvailable;
};

} // namespace graphs

#endif
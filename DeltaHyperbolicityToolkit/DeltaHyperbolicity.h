/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_DELTA_HYPERBOLICITY_H
#define DELTAHYPER_DELTA_HYPERBOLICITY_H

#include "defs.h"
#include "State.h"

namespace dhtoolkit
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
	DeltaHyperbolicity(delta_t delta);

	/*
	 * @param	delta	The delta hyperbolicity value.
	 * @param	nodes	The nodes that convey the delta value.
	 * @note	No check is made to make sure the nodes given in face produce the given delta value.
	 */
	DeltaHyperbolicity(delta_t delta, node_quad_t nodes);

	/*
	 * @returns	The delta value represented by this instance.
	 */
	delta_t getDelta() const;

	/*
	 * @returns	The nodes that produce this instance's delta value.
	 * @throws	DataUnavailableException	If this instance was created without specifying these nodes.
	 */
	const node_quad_t& getNodes() const;

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

} // namespace dhtoolkit

#endif
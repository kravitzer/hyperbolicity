/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_DELTA_HYPERBOLICITY_H
#define DELTAHYPER_DELTA_HYPERBOLICITY_H

#include "defs.h"
#include "State.h"
#include <string>

namespace dhtoolkit
{

/*
 * @brief	Represents a delta hyperbolicity result.
 */
class DeltaHyperbolicity
{
public:
	/*
	 * @brief	Default ctor.
	 */
	DeltaHyperbolicity();

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
	 * @brief	Sets the current instance's delta.
	 */
	void setDelta(delta_t delta);

	/*
	 * @returns	The state that produces this instance's delta value.
	 * @throws	DataUnavailableException	If this instance was created without specifying these nodes.
	 */
	const node_quad_t& getState() const;

	/*
	 * @brief	Sets the current instance's state.
	 */
	void setState(const node_quad_t& state);

	/*
	 * @returns	A textual representation of the nodes this delta represents, e.g.: "3, 110, 166, 201".
	 */
	std::string printNodes() const;

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
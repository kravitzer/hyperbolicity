/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#pragma once

#include "defs.h"
#include "State.h"
#include <string>

namespace hyperbolicity
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
	DeltaHyperbolicity(delta_t delta, const node_combination_t& nodes);

	/*
	 * @brief	Copy ctor - leave compiler's default implementation.
	 */
	//DeltaHyperbolicity(const DeltaHyperbolicity& other);

	/*
	 * @brief	Move ctor.
	 */
	DeltaHyperbolicity(DeltaHyperbolicity&& other);

	/*
	 * @brief	Assignment operator. Makes use of copy ctor and copy-and-swap idiom (therefore parameter is not const-reference).
	 */
	DeltaHyperbolicity& operator=(DeltaHyperbolicity other);

	/*
	 * @returns	The delta value represented by this instance.
	 */
	delta_t getDelta() const;

	/*
	 * @returns	The state that produces this instance's delta value.
	 * @throws	DataUnavailableException	If this instance was created without specifying these nodes.
	 */
	const node_combination_t& getState() const;

	/*
	 * @brief	Sets the current instance's delta & state.
	 */
	void set(delta_t delta, const node_combination_t& state);

	/*
	 * @returns	A textual representation of the nodes this delta represents, e.g.: "3, 110, 166, 201".
	 */
	std::string printNodes() const;

	/*
	 * @brief	Swaps the contents of the two given instances.
	 */
	friend void swap(DeltaHyperbolicity& first, DeltaHyperbolicity& second);

private:
	//the delta hyperbolicity value
	delta_t _delta;
	//the nodes that give the current delta hyperbolicity value
	node_combination_t _nodes;
	//were the nodes specified upon construction? if not, this value will be false, and an exception
	//will be thrown if the user tries to access these nodes.
	bool _nodesAvailable;
};

} // namespace hyperbolicity

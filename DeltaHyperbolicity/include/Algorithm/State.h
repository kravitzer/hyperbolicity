/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#pragma once

#include "defs.h"
#include <string>

namespace hyperbolicity
{
	/*
	 * @brief	A class representing a given state from which a delta hyperbolicity value may be calculated.
	 */
	class State
	{
	public:
		/*
		 * @brief	Default ctor, the instance is unusable, makes the rest of the methods throw std::exception.
		 */
		State();

		/*
		 * @brief	Ctor receiving the state's nodes.
		 * @param	n_i		The i-th node.
		 * @throws	std::exception If the same node is inserted twice (or more).
		 */
		State(const node_ptr_t n1, const node_ptr_t n2, const node_ptr_t n3, const node_ptr_t n4);

		/*
		 * @brief	Resets the state to the given nodes.
		 * @param	n_i		The i-th node.
		 * @throws	std::exception If the same node is inserted twice (or more).
		 */
		void reset(const node_ptr_t n1, const node_ptr_t n2, const node_ptr_t n3, const node_ptr_t n4);

		/*
		 * @brief	Trivial copy ctor & assignment operator. (Default implementation)
		 */
		//State(const State& other);
		//State& operator=(const State& other);

		/*
		 * @returns	The node at position index.
		 * @throws	std::out_of_range	If the given index is out of bounds.
		 */
		const node_ptr_t& operator[](const int index) const;

		/*
		 * @returns	The node at position index.
		 * @throws	std::out_of_range	If the given index is out of bounds.
		 */
		node_ptr_t& operator[](const int index);

		/*
		 * @returns	The number of nodes stored in the state.
		 */
		static unsigned int size();

		/*
		 * @returns	True if state nodes are unique, false otherwise.
		 */
		static bool isStateValid(const node_ptr_t v1, const node_ptr_t v2, const node_ptr_t v3, const node_ptr_t v4);

		/*
		 * @returns	Whether or not this state instance is initialized with nodes.
		 */
		bool isInitialized() const;

		/*
		 * @returns	A textual representation of the nodes this delta represents, e.g.: "3, 110, 166, 201".
		 */
		std::string printNodes() const;

	private:
		static const int NumberOfNodes = 4;
		node_ptr_t _nodes[NumberOfNodes];
	};
} // namespace hyperbolicity

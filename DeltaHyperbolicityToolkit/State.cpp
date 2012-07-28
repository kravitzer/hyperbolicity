#include "State.h"
#include "Except.h"

namespace graphs
{
	State::State()
	{
		//empty
	}

	State::State(const node_ptr_t n1, const node_ptr_t n2, const node_ptr_t n3, const node_ptr_t n4)
	{
		_nodes[0] = n1;
		_nodes[1] = n2;
		_nodes[2] = n3;
		_nodes[3] = n4;

		if ( (_nodes[1] == _nodes[0]) ||
			 (_nodes[2] == _nodes[0]) ||
			 (_nodes[3] == _nodes[0]) ||
			 (_nodes[2] == _nodes[1]) ||
			 (_nodes[3] == _nodes[1]) ||
			 (_nodes[3] == _nodes[2]) ) throw std::exception("State nodes are not unique");
	}

	const node_ptr_t& State::operator[](int index) const
	{
		if ( (index < 0) || (index >= static_cast<int>(size())) ) throw OutOfBoundsException();
		if (nullptr == _nodes[index].get()) throw std::exception("State node is not initialized");
		return _nodes[index];
	}

	node_ptr_t& State::operator[](int index)
	{
		return _nodes[index];
	}

	unsigned int State::size()
	{
		return NumberOfNodes;
	}

} // namespace graphs
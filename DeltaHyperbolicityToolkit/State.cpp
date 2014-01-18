#include "State.h"
#include "Except.h"
#include "Node.h"
#include <string>
#include <boost\format.hpp>

using namespace std;

namespace dhtoolkit
{
	State::State()
	{
		//empty
	}

	State::State(const node_ptr_t n1, const node_ptr_t n2, const node_ptr_t n3, const node_ptr_t n4)
	{
		reset(n1, n2, n3, n4);
	}

	void State::reset(const node_ptr_t n1, const node_ptr_t n2, const node_ptr_t n3, const node_ptr_t n4)
	{
		_nodes[0] = n1;
		_nodes[1] = n2;
		_nodes[2] = n3;
		_nodes[3] = n4;
	}

	const node_ptr_t& State::operator[](int index) const
	{
		if ( (index < 0) || (index >= static_cast<int>(size())) ) throw OutOfBoundsException();
		if (nullptr == _nodes[index].get()) throw std::exception("State node is not initialized");
		return _nodes[index];
	}

	node_ptr_t& State::operator[](int index)
	{
		if ( (index < 0) || (index >= static_cast<int>(size())) ) throw OutOfBoundsException();
		return _nodes[index];
	}

	unsigned int State::size()
	{
		return NumberOfNodes;
	}

	bool State::isStateValid(const node_ptr_t v1, const node_ptr_t v2, const node_ptr_t v3, const node_ptr_t v4)
	{
		return !( (v1 == v2) ||
			      (v1 == v3) ||
				  (v1 == v4) ||
				  (v2 == v3) ||
				  (v2 == v4) ||
				  (v3 == v4) );
	}

	bool State::isInitialized() const
	{
		return (_nodes[0] != nullptr && _nodes[1] != nullptr && _nodes[2] != nullptr && _nodes[3] != nullptr);;
	}

	string State::printNodes() const
	{
		if (!isInitialized()) throw std::exception("State instance is not initialized");

		string res;

		for (unsigned int i = 0; i < NumberOfNodes; ++i)
		{
			res += _nodes[i]->getLabel() + " ";
		}

		return res.substr(0, res.size()-1);
	}

} // namespace dhtoolkit
#include "Cohen.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include "DeltaHyperbolicityToolkit\GraphAlgorithms.h"
#include "DeltaHyperbolicityToolkit\State.h"
#include "DeltaHyperbolicityToolkit\Except.h"
#include "DeltaHyperbolicityToolkit\NodeDistances.h"
#include <string>
#include <vector>

using namespace std;

namespace dhtoolkit
{
	Cohen::Cohen(const string& outputDir) : IGraphAlg(outputDir), _bestDelta(0)
	{
		//empty
	}

	Cohen::~Cohen()
	{
		//empty
	}

	bool Cohen::isComplete() const
	{
        return (_end == _l1);
	}

	void Cohen::initImpl(const node_quad_t&)
	{
		_distances.clear();
		_pairs.clear();
		_bestDelta = 0;

        //find distances and add to array in a sorted manner
		for (unsigned int i = 0; i < _graph->size(); ++i)
        {
            node_ptr_t cur = _graph->getNode(i);
            NodeDistances curDistances(_graph, cur);
            _distances[i] = curDistances.getDistances();
            for (distance_dict_t::const_iterator it = _distances[i].cbegin(); it != _distances[i].cend(); ++it)
            {
                if (it->first > i) addToArray(i, it);
            }
        }

        //initialize iterators
        _l1 = _l2 = _pairs.size()-1;
        _l1Pos = 0;
		_l2Pos = 0;
		advancePointers();
        _end = 0;
	}

	DeltaHyperbolicity Cohen::stepImpl()
	{
        node_quad_t nodes(_graph->getNode(_pairs[_l1].at(_l1Pos).first), _graph->getNode(_pairs[_l1].at(_l1Pos).second), _graph->getNode(_pairs[_l2].at(_l2Pos).first), _graph->getNode(_pairs[_l2].at(_l2Pos).second));
        distance_t d1 = static_cast<distance_t>(_l1 + _l2);
        distance_t d2 = _distances[_pairs[_l1].at(_l1Pos).first][_pairs[_l2].at(_l2Pos).first] + _distances[_pairs[_l1].at(_l1Pos).second][_pairs[_l2].at(_l2Pos).second];
        distance_t d3 = _distances[_pairs[_l1].at(_l1Pos).first][_pairs[_l2].at(_l2Pos).second] + _distances[_pairs[_l1].at(_l1Pos).second][_pairs[_l2].at(_l2Pos).first];
        delta_t delta = GraphAlgorithms::CalculateDeltaFromDistances(d1, d2, d3);

        if (delta > _bestDelta)
        {
            _bestDelta = delta;
            _end = static_cast<size_t>(_bestDelta*2);
        }

        //advance pointers to be ready for next step
        advancePointers();

        return DeltaHyperbolicity(delta, nodes);
	}

    void Cohen::addToArray(const node_index_t origin, const distance_dict_t::const_iterator& it)
    {
        if (_pairs.size() <= it->second)
        {
            _pairs.resize(it->second + 1);
        }

        pair<node_index_t, node_index_t> pairToAdd(origin, it->first);

        vector<pair<node_index_t, node_index_t>>& pairCollection = _pairs[it->second];
        int32_t binaryLowerBound = -1, binaryUpperBound = static_cast<int32_t>(pairCollection.size());
        while (binaryLowerBound < binaryUpperBound - 1)
        {
            size_t binarySearchLocation = (binaryUpperBound + binaryLowerBound) / 2;
            if ( (pairCollection[binarySearchLocation].first < pairToAdd.first) ||
                 (pairCollection[binarySearchLocation].first == pairToAdd.first && (pairCollection[binarySearchLocation].second < pairToAdd.second) ) )
            {
                binaryLowerBound = static_cast<int32_t>(binarySearchLocation);
            }
            else
            {
                binaryUpperBound = static_cast<int32_t>(binarySearchLocation);
            }
        }
        
        //add element before upper bound
        pairCollection.insert(pairCollection.cbegin()+binaryUpperBound, pairToAdd);
    }

	void Cohen::advancePointers()
	{
		do
		{
			++_l2Pos;
			while (_l2Pos == _pairs[_l2].size())
			{
				++_l1Pos;
				while (_l1Pos == _pairs[_l1].size() || ( _l1 == _l2 && _l1Pos == _pairs[_l1].size()-1 ))
				{
					--_l2;
					while (_l2 < _l1)
					{
						--_l1;
						_l2 = _pairs.size() - 1;
					}
					_l1Pos = 0;
				}
            
				if (_l2 != _l1)
				{
					_l2Pos = 0;
				}
				else
				{
					_l2Pos = _l1Pos + 1;
				}
			}
		} while (_pairs[_l1].size() <= _l1Pos || _pairs[_l2].size() <= _l2Pos);
	}



	IGraphAlg* CreateAlgorithm(const string& outputDir)
	{
		IGraphAlg* alg = new Cohen(outputDir);
		return alg;
	}

	void ReleaseAlgorithm(IGraphAlg* alg)
	{
		if (alg) delete alg;
	}
} // namespace dhtoolkit
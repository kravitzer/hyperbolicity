#include "AlgRunner.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include <Windows.h>
#include <memory>
#include <string>
#include <iostream>

using namespace std;
using namespace graphs;

AlgRunner::AlgRunner(string dllPath)
{
	_dll.reset(LoadLibrary(dllPath.c_str()), &FreeLibrary);
	if (nullptr == _dll.get()) throw std::exception("Failed loading dll");

	_createAlg = reinterpret_cast<AlgCreationMethod>(GetProcAddress(_dll.get(), "CreateAlgorithm"));
	if (nullptr == _createAlg) throw std::exception("Failed to get the algorithm creation method");

	_releaseAlg = reinterpret_cast<AlgReleaseMethod>(GetProcAddress(_dll.get(), "ReleaseAlgorithm"));
	if (nullptr == _releaseAlg) throw std::exception("Failed to get the algorithm release method");

	_algorithm = _createAlg();
}

AlgRunner::~AlgRunner()
{
	try
	{
		_releaseAlg(_algorithm);
	}
	catch (...)
	{
		cout << "Exception caught in alg runner dtor" << endl;
	}
}

DeltaHyperbolicity AlgRunner::run(const graph_ptr_t graph) const
{
	return _algorithm->run(graph);
}

DeltaHyperbolicity AlgRunner::runWithInitialState(const graph_ptr_t graph, const node_quad_t& state) const
{
	return _algorithm->runWithInitialState(graph, state);
}
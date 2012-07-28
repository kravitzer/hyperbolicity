#include "AlgRunner.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include <Windows.h>
#include <memory>
#include <string>
#include <iostream>

using namespace std;
using namespace graphs;

AlgRunner::AlgRunner(string dllPath) : _dllPath(dllPath)
{
	//empty
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

void AlgRunner::load()
{
	//do nothing if already loaded
	if (nullptr != _dll.get()) return;

	//load the dll
	_dll.reset(LoadLibrary(_dllPath.c_str()), &FreeLibrary);
	if (nullptr == _dll.get()) throw std::exception("Failed loading dll");

	//get the creation / release methods
	_createAlg = reinterpret_cast<AlgCreationMethod>(GetProcAddress(_dll.get(), "CreateAlgorithm"));
	if (nullptr == _createAlg) throw std::exception("Failed to get the algorithm creation method");

	_releaseAlg = reinterpret_cast<AlgReleaseMethod>(GetProcAddress(_dll.get(), "ReleaseAlgorithm"));
	if (nullptr == _releaseAlg) throw std::exception("Failed to get the algorithm release method");

	//create the algorithm instance
	_algorithm = _createAlg();
}

DeltaHyperbolicity AlgRunner::run(const graph_ptr_t graph) const
{
	if (nullptr == _dll.get()) throw std::exception("Dll must be loaded first");
	return _algorithm->run(graph);
}

DeltaHyperbolicity AlgRunner::runWithInitialState(const graph_ptr_t graph, const node_quad_t& state) const
{
	if (nullptr == _dll.get()) throw std::exception("Dll must be loaded first");
	return _algorithm->runWithInitialState(graph, state);
}
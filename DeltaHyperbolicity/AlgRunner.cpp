#include "AlgRunner.h"
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include <Windows.h>
#include <memory>
#include <string>
#include <iostream>

using namespace std;
using namespace dhtoolkit;

AlgRunner::AlgRunner(const string& dllPath, const string& outputDir) : _dllPath(dllPath), _outputDir(outputDir), _algorithm(nullptr), _createAlg(nullptr), _releaseAlg(nullptr)
{
	//empty
}

AlgRunner::~AlgRunner()
{
	try
	{
		if (_releaseAlg && _algorithm) 
		{
			_releaseAlg(_algorithm);
		}
		else
		{
			if (_algorithm) cout << "WARNING: Algorithm has no release method!" << endl;
		}
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
	_algorithm = _createAlg(_outputDir);
}

DeltaHyperbolicity AlgRunner::step() const
{
	if (nullptr == _dll.get()) throw std::exception("Dll must be loaded first");
	return _algorithm->step();
}

bool AlgRunner::isComplete() const
{
	if (nullptr == _dll.get()) throw std::exception("Dll must be loaded first");
	return _algorithm->isComplete();
}

void AlgRunner::initialize(const graph_ptr_t graph, const node_quad_t& initialState /* = dhtoolkit::node_quad_t() */)
{
	if (nullptr == _dll.get()) throw std::exception("Dll must be loaded first");
	_algorithm->initialize(graph, initialState);
}

string AlgRunner::getName() const
{
	return _dllPath;
}	
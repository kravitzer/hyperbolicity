#include "AlgRunner.h"
#include "Graph\defs.h"
#include "Graph\DeltaHyperbolicity.h"
#include <Windows.h>
#include <memory>
#include <string>
#include <iostream>

using namespace std;
using namespace dhtoolkit;

AlgRunner::AlgRunner(const string& libraryPath) : _libraryPath(libraryPath), _libraryHandle(nullptr), _algName(getNameFromPath(libraryPath)), _algorithm(nullptr), _createAlg(nullptr), _releaseAlg(nullptr)
{
	//load the shared library
	_libraryHandle.reset(LoadLibraryA(_libraryPath.c_str()), &FreeLibrary);
	if (nullptr == _libraryHandle.get()) throw std::exception("Failed loading the shared library");

	//get the creation / release methods
	_createAlg = reinterpret_cast<AlgCreationMethod>(GetProcAddress(_libraryHandle.get(), "CreateAlgorithm"));
	if (nullptr == _createAlg) throw std::exception("Failed to get the algorithm creation method");
	_releaseAlg = reinterpret_cast<AlgReleaseMethod>(GetProcAddress(_libraryHandle.get(), "ReleaseAlgorithm"));
	if (nullptr == _releaseAlg) throw std::exception("Failed to get the algorithm release method");

	//create the algorithm instance
	_algorithm = _createAlg();
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

DeltaHyperbolicity AlgRunner::step() const
{
	return _algorithm->step();
}

bool AlgRunner::isComplete() const
{
	return _algorithm->isComplete();
}

void AlgRunner::initialize(const graph_ptr_t graph, const node_combination_t& initialState /* = dhtoolkit::node_combination_t() */)
{
	_algorithm->initialize(graph, initialState);
}

string AlgRunner::getName() const
{
	return _algName;
}	

string AlgRunner::getNameFromPath(const string& path) const
{
#ifdef _WIN32
	const char PathSeparator = '\\';
#else
	const char PathSeparator = '/';
#endif

	//remove path (i.e. everything up to last path separator, if any exists)
	string name = path;
	size_t lastSlash = name.find_last_of(PathSeparator);
	if (string::npos != lastSlash) name = name.substr(lastSlash + 1);

	//remove extension (i.e. everything after last '.', if any exists)
	size_t ext = name.find_last_of('.');
	if (string::npos != ext) name = name.substr(0, ext);

	return name;
}
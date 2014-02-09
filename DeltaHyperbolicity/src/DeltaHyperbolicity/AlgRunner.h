/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#pragma once

#include <string>
#include <Windows.h>
#include "Graph\defs.h"
#include "DeltaHyperbolicity.h"
#include "IGraphAlg.h"

typedef hyperbolicity::IGraphAlg* (*AlgCreationMethod)();
typedef void (*AlgReleaseMethod)(hyperbolicity::IGraphAlg*);

class AlgRunner
{
public:
	/*
	 * @brief	Loads the shared library & instantiates the algorithm instance.
	 * @param	libraryPath		The path of the shared library to be loaded.
	 * @throws	std::exception	Upon any error (missing library file / expected methods).
	 */
	AlgRunner(const std::string& libraryPath);

	/*
	 * @brief	Dtor, releases algorithm instance & shared library.
	 */ 
	~AlgRunner();

	/*
	 * @brief	Initializes the loaded algorithm.
	 * @param	graph			The graph to run on.
	 * @param	initialState	The initial state for the algorithm to start running from (relevant only to some algortihms, others will ignore this parameter).
	 *							May be left unspecified.
	 */
	void initialize(const hyperbolicity::graph_ptr_t graph, const hyperbolicity::node_combination_t& initialState = hyperbolicity::node_combination_t());

	/*
	 * @brief	Wrapper for running a single step of the algorithm. See IGraphAlg documentation for details.
	 */
	hyperbolicity::DeltaHyperbolicity step() const;

	/*
	 * @returns	True if the algorithm has completed its run, false otherwise.
	 */
	bool isComplete() const;

	/*
	 * @returns	The algorithm's name.
	 */
	std::string getName() const;

private:
	//do *not* allow copy ctor / assignment operator
	AlgRunner(const AlgRunner&);
	AlgRunner& operator=(const AlgRunner&);

	/*
	 * @brief	Extracts the file name from its path, e.g. "C:\algorithms\alg1.dll" returns "alg1".
	 * @param	path	The path to the algorithm name (either relative or full).
	 * @returns	The file name only, no extension or path.
	 */
	std::string getNameFromPath(const std::string& path) const;

	//the shared library to be loaded
	std::string _libraryPath;

	//a descriptive name of the algorithm
	std::string _algName;

	//handle to the loaded algorithm library
	std::shared_ptr<HINSTANCE__> _libraryHandle;

	//creation & release method pointers
	AlgCreationMethod _createAlg;
	AlgReleaseMethod _releaseAlg;

	//algorithm instance
	hyperbolicity::IGraphAlg* _algorithm;
};

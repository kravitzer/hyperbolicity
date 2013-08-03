/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_ALGRUNNER_H
#define DELTAHYPER_ALGRUNNER_H

#include <string>
#include <Windows.h>
#include "DeltaHyperbolicityToolkit\defs.h"
#include "DeltaHyperbolicityToolkit\DeltaHyperbolicity.h"
#include "DeltaHyperbolicityToolkit\IGraphAlg.h"

typedef dhtoolkit::IGraphAlg* (*AlgCreationMethod)(const std::string& outputDir);
typedef void (*AlgReleaseMethod)(dhtoolkit::IGraphAlg*);

class AlgRunner
{
public:
	/*
	 * @param	dllPath		The dll to be loaded.
	 */
	AlgRunner(const std::string& dllPath, const std::string& outputDir);

	/*
	 * @brief	Dtor, releases algorithm instance.
	 */ 
	~AlgRunner();

	/*
	 * @brief	Loads the dll & instantiates the algorithm instance.
	 * @throws	std::exception	Upon any error (missing dll / expected methods).
	 */
	void load();

	/*
	 * @brief	Initializes the loaded algorithm.
	 * @param	graph	The graph to run on.
	 * @param	initialState	The initial state for the algorithm to start running from (relevant only to some algortihms, others will ignore this parameter).
	 *							May be left unspecified.
	 */
	void initialize(const dhtoolkit::graph_ptr_t graph, const dhtoolkit::node_quad_t& initialState = dhtoolkit::node_quad_t());

	/*
	 * @brief	Wrapper for running a single step of the algorithm. See IGraphAlg documentation for details.
	 */
	dhtoolkit::DeltaHyperbolicity step() const;

	/*
	 * @returns	True if the algorithm has completed its run, false otherwise.
	 */
	bool isComplete() const;

	/*
	 * @returns	The algorithm's name.
	 */
	std::string getName() const;

	/*
	 * @returns	The output dir.
	 */
	std::string getOutputDir() const { return _outputDir; }

private:
	//do *not* allow copy ctor / assignment operator
	AlgRunner(const AlgRunner&);
	AlgRunner& operator=(const AlgRunner&);

	//the dll to be loaded
	std::string _dllPath;

	//the output directory
	std::string _outputDir;

	//handle to the loaded dll
	std::shared_ptr<HINSTANCE__> _dll;

	//creation & release method pointers
	AlgCreationMethod _createAlg;
	AlgReleaseMethod _releaseAlg;

	//algorithm instance
	dhtoolkit::IGraphAlg* _algorithm;
};

#endif // DELTAHYPER_ALGRUNNER_H


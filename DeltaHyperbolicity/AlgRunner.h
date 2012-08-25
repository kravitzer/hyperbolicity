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
	 * @brief	Wrapper for running the algorithm. See IGraphAlg documentation for details.
	 */
	dhtoolkit::DeltaHyperbolicity run(const dhtoolkit::graph_ptr_t graph) const;

	/*
	 * @brief	Wrapper for running the algorithm. See IGraphAlg documentation for details.
	 */
	dhtoolkit::DeltaHyperbolicity runWithInitialState(const dhtoolkit::graph_ptr_t graph, const dhtoolkit::node_quad_t& state) const;

	/*
	 * @returns	The algorithm's name.
	 */
	std::string getName() const;

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


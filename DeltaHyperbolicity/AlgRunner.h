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

typedef graphs::IGraphAlg* (*AlgCreationMethod)();
typedef void (*AlgReleaseMethod)(graphs::IGraphAlg*);

class AlgRunner
{
public:
	/*
	 * @brief	Loads the given dll & instantiates the algorithm instance.
	 * @throws	std::exception	Upon any error (missing dll / expected methods).
	 */
	AlgRunner(std::string dllPath);

	/*
	 * @brief	Dtor, releases algorithm instance.
	 */ 
	~AlgRunner();

	/*
	 * @brief	Wrapper for running the algorithm. See IGraphAlg documentation for details.
	 */
	graphs::DeltaHyperbolicity run(const graphs::graph_ptr_t graph) const;

	/*
	 * @brief	Wrapper for running the algorithm. See IGraphAlg documentation for details.
	 */
	graphs::DeltaHyperbolicity runWithInitialState(const graphs::graph_ptr_t graph, const graphs::node_quad_t& state) const;

private:
	//do *not* allow copy ctor / assignment operator
	AlgRunner(const AlgRunner&);
	AlgRunner& operator=(const AlgRunner&);

	//handle to the loaded dll
	std::shared_ptr<HINSTANCE__> _dll;

	//creation & release method pointers
	AlgCreationMethod _createAlg;
	AlgReleaseMethod _releaseAlg;

	//algorithm instance
	graphs::IGraphAlg* _algorithm;
};

#endif // DELTAHYPER_ALGRUNNER_H

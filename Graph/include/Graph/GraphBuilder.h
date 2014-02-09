#pragma once

#include "defs.h"

namespace hyperbolicity
{

/*
 * @brief	Helper class that generates different type of graphs.
 */
class GraphBuilder
{
public:
	static graph_ptr_t createERGraph(unsigned int nodeCount, double p);
	static graph_ptr_t createBAGraph(unsigned int n, double m);
	static graph_ptr_t createBAExtendedGraph(unsigned int n, double m, double p);

private:
	static unsigned int calculateNextM(double m);
};

} // namespace hyperbolicity
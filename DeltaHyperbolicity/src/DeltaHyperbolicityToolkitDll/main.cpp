#include "Graph\defs.h"
#include "Graph\Graph.h"

using namespace dhtoolkit;

extern "C" __declspec(dllexport) Graph* GraphCreate(const char* title)
{
	return new Graph(title);
}

extern "C" __declspec(dllexport) void GraphDestroy(Graph* graph)
{
	delete graph;
}

extern "C" __declspec(dllexport) node_index_t GraphInsertNode(Graph* graph, const char* label)
{
	return graph->insertNode(label)->getIndex();
}

extern "C" __declspec(dllexport) size_t GraphSize(Graph* graph)
{
	return graph->size();
}

extern "C" __declspec(dllexport) size_t GraphEdgeCount(Graph* graph)
{
	return graph->edgeCount();
}

extern "C" __declspec(dllexport) void GraphRemoveNode(Graph* graph, node_index_t nodeIndex)
{
	return graph->removeNode(nodeIndex);
}


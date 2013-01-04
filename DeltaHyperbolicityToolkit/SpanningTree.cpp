#include "SpanningTree.h"
#include "Graph.h"
#include "Node.h"

namespace dhtoolkit
{

SpanningTree::SpanningTree(const graph_ptr_t graph, const node_ptr_t root) : BFS(graph), _tree( new Graph(graph->getTitle()) )
{
	//insert as many nodes as the graph has, no edges yet
	for (unsigned int i = 0; i < _graph->size(); ++i)
	{
		_tree->insertNode();
	}

	//run bfs
	run(root);
}

SpanningTree::~SpanningTree()
{
	//empty on purpose
}

graph_ptr_t SpanningTree::getTree() const
{
	return _tree;
}

bool SpanningTree::nodeTraversal(const node_ptr_t curNode, const node_ptr_t prevNode, distance_t distanceFromOrigin)
{
	//insert edge b/w corresponding nodes in our graph as well
	_tree->getNode( curNode->getIndex() )->insertBidirectionalEdgeTo( _tree->getNode( prevNode->getIndex() ) );
	return true;
}

} // namespace dhtoolkit
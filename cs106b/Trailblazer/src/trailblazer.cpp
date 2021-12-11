/* trailblazer.cpp
 * ---------------
 * By: Juan Planas (TA: Eric Yu)
 * Implentation of various path-finding algorithms and of Kruskal's algorithm
 * to generate mazes.
 */

#include "trailblazer.h"
#include "pqueue.h"
#include "queue.h"
#include "hashset.h"
#include "map.h"

using namespace std;


/* Recursive helper fucntion for depth first search.
 * Assumes properly initialized graph and path, and valid vertices.
 * Stores the path it follows between vertices in path, with an empty vector if
 * no valid path is found.
 */
bool recurseDfs(BasicGraph &graph, Vertex *v1, Vertex *v2, Vector<Vertex*> &path) {
	path.add(v1);
	v1->visited = true;
	v1->setColor(GREEN);

	if (v1 == v2) {
		//Path found
		return true;
	}

	for (Vertex *neighbor : graph.getNeighbors(v1)) {
		if (!neighbor->visited && recurseDfs(graph, neighbor, v2, path)) {
			return true;
		}
	}
	path.remove(path.size() - 1);
	v1->setColor(GRAY);
	return false;
}

/* Implements depth first search between two vertices belonging to the provided
 * graph. Returns a path between start and end, and returns an empty vector if
 * no path is found.
 * Assumes initialized graph and valid start and end vertices.
 */
Vector<Vertex*> depthFirstSearch(BasicGraph& graph, Vertex* start, Vertex* end) {
	graph.resetData();	
    Vector<Vertex*> path;

	recurseDfs(graph, start, end, path);

    return path;
}

/* Follows previous pointers from the last vertex back to the first vertex in a
 * path found by BFS, Dijkstra's, or A*, and adds the path followed to path.
 * Assumes valid path vector and end vertex.
 */
void rebuildPath(Vertex *end, Vector<Vertex*> &path) {
	while (end != NULL) {
		end->setColor(GREEN);
		path.insert(0, end);
		end = end->previous;
	}
}

/* Implementation of breadth first search, returns the path found between the
 * start and end vertices.
 * Assumes a valid graph and valid start and end vertices (ie, the pointers
 * point to valid/initialized Vertex objects).
 */
Vector<Vertex*> breadthFirstSearch(BasicGraph& graph, Vertex* start, Vertex* end) {
	Queue<Vertex*> queue;
    Vector<Vertex*> path;
	graph.resetData();	

	queue.enqueue(start);
	start->visited = true;
	start->setColor(GREEN);
	while (!queue.isEmpty()) {
		Vertex *v = queue.dequeue();
		v->setColor(GREEN);
		if (v == end) {
			rebuildPath(end, path);
			break;
		}
		for (Vertex *neighbor : graph.getNeighbors(v)) {
			if (neighbor->visited) {
				continue;
			}
			neighbor->visited = true;
			neighbor->setColor(YELLOW);
			neighbor->previous = v;
			queue.enqueue(neighbor);
		}
	}

    return path;
}

/* Initializes all vertices in graph to have infinite cost and be unvisited.
 * Assumes a valid graph.
 */
void initVertices(BasicGraph &graph) {
	for (Vertex *v : graph.getVertexSet()) {
		v->cost = POSITIVE_INFINITY;
		v->visited = false;
	}
}

/* Implementation of Dijkstra's algorithm, returns a vector containing the
 * vertices visited in order from start to end.
 * Assumes an initialized graph and valid start and end vertices.
 */
Vector<Vertex*> dijkstrasAlgorithm(BasicGraph& graph, Vertex* start, Vertex* end) {
	graph.resetData();	

    Vector<Vertex*> path;
	PriorityQueue<Vertex*> queue;
	initVertices(graph);

	start->cost = 0;
	queue.enqueue(start, 0);

	while (!queue.isEmpty()) {
		Vertex *v = queue.dequeue();
		v->visited = true;
		v->setColor(GREEN);
		if (v == end) {
			rebuildPath(end, path);
			break;
		}
		for (Vertex *neighbor : graph.getNeighbors(v)) {
			if (neighbor->visited) {
				continue;
			}
			double cost = v->cost + graph.getEdge(v, neighbor)->cost;

			if (cost < neighbor->cost) {
				neighbor->cost = cost;
				neighbor->setColor(YELLOW);
				neighbor->previous = v;
				if (neighbor->getColor() != YELLOW) {
					queue.changePriority(neighbor, cost);
				} else {
					queue.enqueue(neighbor, cost);
				}
			}
		}
	}

    return path;
}

/* Implementation of A* search algorithm. The function returns a vector
 * containing the vertices in the path in order from start to end.
 * Assumes an initialized graph and valid start and end vertices.
 */
Vector<Vertex*> aStar(BasicGraph& graph, Vertex* start, Vertex* end) {
	graph.resetData();	
    Vector<Vertex*> path;
	PriorityQueue<Vertex*> queue;
	initVertices(graph);

	double h = heuristicFunction(start, end);

	start->cost = 0;
	queue.enqueue(start, h);

	while (!queue.isEmpty()) {
		Vertex *v = queue.dequeue();
		v->visited = true;
		v->setColor(GREEN);
		if (v == end) {
			//For some reason, if there is a non-zero delay in the GUI the path
			//sometimes fails to display correctly. It's correct in the path
			//vector, however.
			rebuildPath(end, path);
			break;
		}
		for (Vertex *neighbor : graph.getNeighbors(v)) {
			if (neighbor->visited) {
				continue;
			}
			double cost = v->cost + graph.getEdge(v, neighbor)->cost;

			if (cost < neighbor->cost) {
				neighbor->cost = cost;
				neighbor->setColor(YELLOW);
				neighbor->previous = v;
				h = heuristicFunction(neighbor, end);
				if (neighbor->getColor() != YELLOW) {
					queue.changePriority(neighbor, cost + h);
				} else {
					queue.enqueue(neighbor, cost + h);
				}
			}
		}
	}

    return path;
}

/* Generate a minimum spanning tree of the given graph using Kruskal's
 * algorithm. A Set of the edges composing the MST is returned.
 * Assumes an initialized graph.
 */
Set<Edge*> kruskal(BasicGraph& graph) {
    Set<Edge*> mst;
	PriorityQueue<Edge*> queue;

	//Initialization of PQueue and clusters
	for (Edge *e : graph.getEdgeSet()) {
		queue.enqueue(e, e->cost);
	}

	//Pointers to HashSets are used to speed up copying/union operations.
	Map<Vertex*, HashSet<Vertex*>* > clusters;
	for (Vertex *v : graph.getVertexSet()) {
		HashSet<Vertex*> *temp = new HashSet<Vertex*>;
		temp->add(v);
		clusters.add(v, temp);
	}

	while (!queue.isEmpty()) {
		Edge *e = queue.dequeue();
		Vertex *v1 = e->start;
		Vertex *v2 = e->finish;

		HashSet<Vertex*> *c1 = clusters.get(v1);
		HashSet<Vertex*> *c2 = clusters.get(v2);

		if (*c1 != *c2) {
			HashSet<Vertex*> *temp = new HashSet<Vertex*>;
			*temp = *c1 + *c2;
			//Make sure the keys of the map from the old clusterall point to
			//the same cluster, and free the old HashSets.
			for (Vertex *vert : *c1) {
				clusters.put(vert, temp);
			}
			for (Vertex *vert : *c2) {
				clusters.put(vert, temp);
			}
			delete c1;
			delete c2;
			mst.add(e);
		}

		//Make sure to get last cluster HashSet
		HashSet<Vertex*> *last = clusters.first();
		delete last;

	}

	return mst;
}

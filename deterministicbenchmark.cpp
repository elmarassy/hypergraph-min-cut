#include "hypergraph.h"
#include <vector>
#include <algorithm>
#include <queue>
#include <boost/heap/fibonacci_heap.hpp>
#include "main.cpp"

struct Task {
    int priority;
    int vertexIndex;

    bool operator<(const Task& other) const {
        return priority < other.priority;
    }
};


std::vector<int> minCutPhase(Hypergraph& H){
    // Equivalent to A in the algorithm. We let A contain the first node.
    std::unordered_set<int> currSubset;
    currSubset.push_back(0);

    std::vector<bool> markedEdges(H.edges.size());
    boost::heap::fibonacci_heap<Task> sortedVertices;
    std::vector<boost::heap::fibonacci_heap<Task>::handle_type> heapHandles;

    // Initialize the weights w(A) for all vertices V/{0} for A = {0}
    for (int i = 1; i < H.vertices.size(); i++){
        Task t;
        t.priority = 0;
        t.vertexIndex = i;
        heapHandles[i-1] = sortedVertices.push(t);
    }

    for(int e: H.vertices[0].incidentEdges){
        markedEdges[e] = true;
        int edgeWeight = H.edges[e].weight;
        for (int v: H.edges[e].vertices){
            Task &t = *heapHandles[v];
            sortedVertices.increase(heapHandles[v], Task{t.priority+edgeWeight, t.vertexIndex});
        }
    }

    std::vector<int> lastVertices(2);

    // Iterate updating the subset and calculating the min-cut
    while (currSubset.size() < H.vertices.size()){
        Task tightestTask = sortedVertices.top();
        int tighestVertex = tightestTask.vertexIndex;
        currSubset.insert(tighestVertex);
        lastAddedVertex[0] = lastAddedVertex[1];
        lastAddedVertex[1] = tighestVertex;

        for(int e: H.vertices[tighestVertex].incidentEdges){
            if (markedEdges[e]) // If an edge is marked, we don't update
                continue;

            markedEdges[e] = true;
            int edgeWeight = H.edges[e].weight;

            for (int v: H.edges[e].vertices){
                Task &t = *heapHandles[v];
                int neighborIndex = t.vertexIndex
                if (currSubset.count(neighborIndex) == 0){
                    sortedVertices.increase(heapHandles[v], Task{t.priority+edgeWeight, neighborIndex});
                }
            }
        }
        sortedVertices.pop();
    }

    return lastVertices;
}

void mergeVertices(Hypergraph& H, int vertex1, int vertex2){
    // First check if any edges in vertex1 is shared by an edge in vertex2
    H.edges.push_back(Hyperedge{1, {vertex1, vertex2}});
    H = contract(H, H.edges.size()-1);
}

int deterministicMinCut(Hypergraph& H){
    int maxMinCut = INT_MAX;
    while (H.vertices.size() > 1){
        // Get phase cut
        std::vector<int> lastVertices = minCutPhase(H);
        int phaseCut = 0;
        for (Hyperedge e: H.vertices[lastVertices[1]].incidentEdges){
            phaseCut += e.weight;
        }
        maxMinCut = std::min(maxMinCut, phaseCut);
        // Contraction Step
        mergeVertices(H, lastVertices[0], lastVertices[1]);
    }
    return maxMinCut;
}

int main() {
    Hypergraph H;
    H.vertices = {
            {{0}},
            {{0, 2}},
            {{0, 1}},
            {{1, 2}},
            {{2}}
    };
    H.edges = {
            {50, {0, 1, 2}},
            {3, {2, 3}},
            {10, {1, 3, 4}}
    };

    deterministicMinCut(H)

}
//#include "deterministichypergraph.h"
#include <vector>
#include <algorithm>
#include <queue>
#include <boost/heap/fibonacci_heap.hpp>
#include <unordered_map>

#include "hypergraph.h"
#include "randomized.h"
#include "deterministicbenchmark.h"

Hypergraph mergeVertices(Hypergraph& H, uint32_t vertex1, uint32_t vertex2){
    // Assign all edges from vertex2 to vertex1
    Hyperedge auxillary;
    auxillary.weight = 0;
    auxillary.vertices = {vertex1, vertex2};
    H.edges.push_back(auxillary);
    return contract(H, H.edges.size()-1);
}

uint32_t minCutPhase(Hypergraph& H){
    // Equivalent to A in the algorithm. We let A contain the first node.
    std::unordered_set<uint32_t> currSubset;
    currSubset.insert(0);

    std::unordered_map<Hyperedge*, bool> markedEdges;
    boost::heap::fibonacci_heap<Task> sortedVertices;
    // Maps the vertex id to its handle in the heap
    std::unordered_map<uint32_t, boost::heap::fibonacci_heap<Task>::handle_type> heapHandles;

    // Generate incident edges because our merging doesn't keep track of it
    std::vector<std::unordered_set<Hyperedge*>> incidentEdges(H.n);
    for (Hyperedge& e: H.edges){
        markedEdges.insert({&e, false});
        for (uint32_t v: e.vertices){
            incidentEdges[v].insert(&e);
        }
    }


    // Initialize the weights w(A) for all vertices V/{0} for A = {0}
    for (uint32_t i = 0; i < H.n; i++){
        Task t;
        t.priority = 0;
        t.vertex = i;
        heapHandles[i] = sortedVertices.push(t);
    }

    // Update A
    for(Hyperedge* e: incidentEdges[0]){
        markedEdges[e] = true;
        int edgeWeight = e->weight;
        for (uint32_t v: e->vertices){
            Task &t = *heapHandles[v];
            sortedVertices.increase(heapHandles[v], Task{t.priority+edgeWeight, v});
        }
    }

    std::vector<uint32_t> lastVertices(2);

    // Iterate updating the subset and calculating the min-cut
    while (currSubset.size() < H.n){
        Task tightestTask = sortedVertices.top();
        sortedVertices.pop();
        uint32_t tightestVertex = tightestTask.vertex;
        currSubset.insert(tightestVertex);
        lastVertices[0] = lastVertices[1];
        lastVertices[1] = tightestVertex;

        for(Hyperedge* e: incidentEdges[tightestVertex]){
            if (markedEdges[e]) // If an edge is marked, we don't update
                continue;

            markedEdges[e] = true;
            uint32_t edgeWeight = e->weight;

            for (uint32_t v: e->vertices){
                Task &t = *heapHandles[v];
                uint32_t neighborVertex = t.vertex;
                if (currSubset.count(neighborVertex) == 0){
                    sortedVertices.increase(heapHandles[neighborVertex], Task{t.priority+edgeWeight, neighborVertex});
                }
            }
        }
    }

    uint32_t phaseCut = 0;
    for (Hyperedge* e: incidentEdges[lastVertices[1]]){
        phaseCut += e->weight;
    }

    H = mergeVertices(H, lastVertices[0], lastVertices[1]);

    return phaseCut;
}

uint32_t deterministicMinCut(Hypergraph& H){
    uint32_t maxMinCut = INT_MAX;
    while (H.n > 1){
        // Get phase cut
        uint32_t phaseCut = minCutPhase(H);
        maxMinCut = std::min(maxMinCut, phaseCut);
    }
    return maxMinCut;
}



/**
struct Task {
    int priority;
    Vertex* vertex;

    bool operator<(const Task& other) const {
        return priority < other.priority;
    }
};


std::vector<Vertex*> minCutPhase(PointerHypergraph& H){
    // Equivalent to A in the algorithm. We let A contain the first node.
    std::unordered_set<Vertex*> currSubset;
    currSubset.insert(&H.vertices[0]);

    std::map<int, bool> markedEdges(H.edges.size());
    boost::heap::fibonacci_heap<Task> sortedVertices;
    // Maps the vertex id to its handle in the heap
    std::map<int, boost::heap::fibonacci_heap<Task>::handle_type> heapHandles;

    // Initialize the weights w(A) for all vertices V/{0} for A = {0}
    for (std::unique_ptr<Vertex> v: H.vertices){
        Task t;
        t.priority = 0;
        t.vertex = &v;
        heapHandles[v->id] = sortedVertices.push(t);
    }

    for(Hyperedge* e: H.vertices[0].incidentEdges){
        markedEdges[e->id] = true;
        int edgeWeight = e->weight;
        for (Vertex* v: e->vertices){
            Task &t = *heapHandles[v->id];
            sortedVertices.increase(heapHandles[v->id], Task{t.priority+edgeWeight, v});
        }
    }

    std::vector<Vertex*> lastVertices(2);

    // Iterate updating the subset and calculating the min-cut
    while (currSubset.size() < H.vertices.size()){
        Task tightestTask = sortedVertices.top();
        sortedVertices.pop();
        Vertex* tighestVertex = tightestTask.vertex;
        currSubset.insert(tighestVertex);
        lastVertices[0] = lastVertices[1];
        lastVertices[1] = tighestVertex;

        for(Hyperedge* e: tighestVertex->incidentEdges){
            if (markedEdges[e]) // If an edge is marked, we don't update
                continue;

            markedEdges[e->id] = true;
            int edgeWeight = e->weight;

            for (Vertex* v: e->vertices){
                Task &t = *heapHandles[v->id];
                Vertex* neighborVertex = t.vertex;
                if (currSubset.count(neighborVertex) == 0){
                    sortedVertices.increase(heapHandles[neighborVertex->id], Task{t.priority+edgeWeight, neighborVertex});
                }
            }
        }
    }

    return lastVertices;
}

void mergeVertices(PointerHypergraph& H, Vertex* vertex1, Vertex* vertex2){
    // Assign all edges from vertex2 to vertex1
    for (Hyperedge* e: vertex2->incidentEdges){
        e->vertices.erase(vertex2);
        e->vertices.insert(vertex1);
        vertex1->incidentEdges.insert(e);
    }

    auto v2index = std::find_if(H.vertices.begin(), H.vertices.end(), [&](auto& ptr){ptr.get() == vertex2;});
    H.vertices.erase(v2index); // Remove vertex2 from the hypergraph

    // Get rid of all edges containing only vertex1
    for(auto iter = H.edges.begin(); iter != H.edges.end();){
        Hyperedge* e = iter->get();
        if(e->vertices.size() == 1){
            Vertex* top = *e->vertices.begin();
            if (top == vertex1){
                iter = H.edges.erase(iter);
                top->incidentEdges.erase(e);
            }
        }
        ++iter;
    }
}

int deterministicMinCut(PointerHypergraph& H){
    int maxMinCut = INT_MAX;
    while (H.vertices.size() > 1){
        // Get phase cut
        std::vector<Vertex*> lastVertices = minCutPhase(H);
        int phaseCut = 0;
        for (Hyperedge* e: lastVertices[1]->incidentEdges){
            phaseCut += e->weight;
        }
        maxMinCut = std::min(maxMinCut, phaseCut);
        // Contraction Step
        mergeVertices(H, lastVertices[0], lastVertices[1]);
    }
    return maxMinCut;
}
*/
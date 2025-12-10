#include <vector>
#include <algorithm>
#include <queue>
#include <boost/heap/fibonacci_heap.hpp>
#include <unordered_map>

#include "hypergraph.h"
#include "randomized.h"

struct Task {
    uint32_t priority;
    uint32_t vertex;

    bool operator<(const Task& other) const {
        return priority < other.priority;
    }
};

Hypergraph mergeVertices(Hypergraph& H, uint32_t vertex1, uint32_t vertex2);

uint32_t minCutPhase(Hypergraph& H);

uint32_t deterministicMinCut(Hypergraph& H);
#ifndef DETERMINISTIC_H
#define DETERMINISTIC_H

#include "hypergraph.h"
#include <chrono>
#include <iostream>
#include <limits>

struct DeterministicResult {
    uint64_t cutWeight = 0;
    uint64_t numPhases = 0;
    uint64_t numVertexMerges = 0;
    uint64_t numEdgeUpdates = 0;
    double timeSeconds = 0.0;
};

struct DeterministicHypergraph {
    uint32_t n;
    std::vector<Hyperedge> edges;
    std::vector<std::vector<uint32_t>> incident;
    std::vector<uint32_t> parent;
    std::vector<uint32_t> rank_;
    uint32_t activeVertices;

    explicit DeterministicHypergraph(const Hypergraph& H)
            : n(H.n), edges(H.edges), incident(H.n), parent(H.n), rank_(H.n, 0), activeVertices(H.n) {

        std::iota(parent.begin(), parent.end(), 0u);

        for (uint32_t ei = 0; ei < edges.size(); ++ei) {
            for (uint32_t v : edges[ei].vertices) {
                incident[v].push_back(ei);
            }
        }
    }

    uint32_t find(uint32_t x) {
        uint32_t root = x;
        while (parent[root] != root) root = parent[root];
        while (parent[x] != root) {
            uint32_t next = parent[x];
            parent[x] = root;
            x = next;
        }
        return root;
    }

    void unite(uint32_t a, uint32_t b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (rank_[a] < rank_[b]) std::swap(a, b);
        parent[b] = a;
        if (rank_[a] == rank_[b]) ++rank_[a];
        --activeVertices;

        for (uint32_t ei : incident[b]) {
            incident[a].push_back(ei);
        }
        incident[b].clear();
    }

    bool isNonTrivial(uint32_t ei) {
        const auto& e = edges[ei];
        if (e.vertices.empty()) return false;
        uint32_t firstRoot = find(e.vertices[0]);
        for (size_t i = 1; i < e.vertices.size(); ++i) {
            if (find(e.vertices[i]) != firstRoot) return true;
        }
        return false;
    }
};

inline std::pair<uint64_t, std::pair<uint32_t, uint32_t>> minCutPhase(
        DeterministicHypergraph& H,
        std::vector<uint64_t>& connectivity,
        std::vector<bool>& inA,
        std::vector<bool>& edgeUsed,
        uint64_t& edgeUpdates
) {
    if (H.activeVertices < 2) {
        return {std::numeric_limits<uint64_t>::max(), {0, 0}};
    }

    std::vector<uint32_t> activeReps;
    activeReps.reserve(H.activeVertices);
    for (uint32_t v = 0; v < H.n; ++v) {
        if (H.find(v) == v) activeReps.push_back(v);
    }

    if (activeReps.size() < 2) {
        return {std::numeric_limits<uint64_t>::max(), {0, 0}};
    }

    for (uint32_t v : activeReps) {
        connectivity[v] = 0;
        inA[v] = false;
    }
    std::fill(edgeUsed.begin(), edgeUsed.end(), false);

    uint32_t current = activeReps[0];
    inA[current] = true;
    uint32_t lastV = current, secondLastV = current;

    for (uint32_t ei : H.incident[current]) {
        if (edgeUsed[ei] || !H.isNonTrivial(ei)) continue;
        edgeUsed[ei] = true;

        uint64_t w = H.edges[ei].weight;
        for (uint32_t v : H.edges[ei].vertices) {
            uint32_t r = H.find(v);
            if (!inA[r]) {
                connectivity[r] += w;
                ++edgeUpdates;
            }
        }
    }

    for (size_t step = 1; step < activeReps.size(); ++step) {
        uint32_t best = 0;
        uint64_t bestConn = 0;
        bool foundFirst = false;

        for (uint32_t v : activeReps) {
            if (!inA[v] && (!foundFirst || connectivity[v] > bestConn)) {
                best = v;
                bestConn = connectivity[v];
                foundFirst = true;
            }
        }

        if (!foundFirst) break;

        secondLastV = lastV;
        lastV = best;
        inA[best] = true;

        for (uint32_t ei : H.incident[best]) {
            if (edgeUsed[ei] || !H.isNonTrivial(ei)) continue;
            edgeUsed[ei] = true;

            uint64_t w = H.edges[ei].weight;
            for (uint32_t v : H.edges[ei].vertices) {
                uint32_t r = H.find(v);
                if (!inA[r]) {
                    connectivity[r] += w;
                    ++edgeUpdates;
                }
            }
        }
    }

    uint64_t cutWeight = 0;
    for (uint32_t ei : H.incident[lastV]) {
        if (H.isNonTrivial(ei)) {
            cutWeight += H.edges[ei].weight;
        }
    }

    return {cutWeight, {secondLastV, lastV}};
}

inline DeterministicResult deterministicMinCut(const Hypergraph& H, bool verbose = false) {

    auto startTime = std::chrono::high_resolution_clock::now();

    DeterministicResult result;
    result.cutWeight = std::numeric_limits<uint64_t>::max();

    if (H.n <= 1 || H.edges.empty()) {
        result.cutWeight = 0;
        return result;
    }

    DeterministicHypergraph FH(H);
    std::vector<uint64_t> connectivity(H.n, 0);
    std::vector<bool> inA(H.n, false);
    std::vector<bool> edgeUsed(H.edges.size(), false);

    while (FH.activeVertices > 1) {
        auto [phaseCut, verticesToMerge] = minCutPhase(FH, connectivity, inA, edgeUsed, result.numEdgeUpdates);

        if (phaseCut < result.cutWeight) {
            result.cutWeight = phaseCut;
        }

        FH.unite(verticesToMerge.first, verticesToMerge.second);
        ++result.numPhases;
        ++result.numVertexMerges;

        if (verbose && result.numPhases % 100 == 0) {
            std::cout << "  Phase " << result.numPhases
                      << ": vertices=" << FH.activeVertices
                      << ", best=" << result.cutWeight << "\n";
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.timeSeconds = std::chrono::duration<double>(endTime - startTime).count();

    return result;
}

inline DeterministicResult deterministicMinKCut(const Hypergraph& H, uint32_t k, bool verbose = false) {
    if (k == 2) return deterministicMinCut(H, verbose);

    auto startTime = std::chrono::high_resolution_clock::now();
    DeterministicResult result;

    Hypergraph current = H;

    for (uint32_t i = 0; i < k - 1 && current.n > 1; ++i) {
        auto cutResult = deterministicMinCut(current, false);
        result.cutWeight += cutResult.cutWeight;
        result.numPhases += cutResult.numPhases;
        result.numVertexMerges += cutResult.numVertexMerges;
        result.numEdgeUpdates += cutResult.numEdgeUpdates;

        if (verbose) std::cout << "  Cut " << (i+1) << ": " << cutResult.cutWeight << "\n";

        uint64_t removed = 0;
        std::vector<Hyperedge> remaining;
        for (auto& e : current.edges) {
            if (removed < cutResult.cutWeight) removed += e.weight;
            else remaining.push_back(std::move(e));
        }
        current.edges = std::move(remaining);
    }

    result.timeSeconds = std::chrono::duration<double>(
            std::chrono::high_resolution_clock::now() - startTime).count();
    return result;
}

#endif

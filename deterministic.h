#pragma once

#include "hypergraph.h"
#include <algorithm>
#include <limits>
#include <numeric>
#include <vector>
#include "randomized.h"

inline uint64_t deterministicMinCut(const Hypergraph& H) {
    if (H.n <= 1 || H.edges.empty()) {
        return 0;
    }

    DSU dsu(H.n);
    uint64_t minCut = std::numeric_limits<uint64_t>::max();
    const uint32_t m = H.edges.size();

    while (dsu.components() > 1) {
        std::vector<uint32_t> reps;
        for (uint32_t v = 0; v < H.n; ++v) {
            if (dsu.find(v) == v) reps.push_back(v);
        }

        if (reps.size() < 2) break;
        std::vector<std::vector<uint32_t>> edgeReps(m);
        std::vector<std::vector<uint32_t>> incident(H.n);

        for (uint32_t ei = 0; ei < m; ++ei) {
            for (uint32_t v : H.edges[ei].vertices) {
                edgeReps[ei].push_back(dsu.find(v));
            }
            std::sort(edgeReps[ei].begin(), edgeReps[ei].end());
            edgeReps[ei].erase(std::unique(edgeReps[ei].begin(), edgeReps[ei].end()),
                               edgeReps[ei].end());

            if (edgeReps[ei].size() >= 2) {
                for (uint32_t r : edgeReps[ei]) {
                    incident[r].push_back(ei);
                }
            }
        }

        std::vector<uint64_t> connectivity(H.n, 0);
        std::vector<bool> inA(H.n, false);
        std::vector<bool> edgeCrossed(m, false);

        inA[reps[0]] = true;

        for (uint32_t ei : incident[reps[0]]) {
            edgeCrossed[ei] = true;
            uint64_t w = H.edges[ei].weight;
            for (uint32_t r : edgeReps[ei]) {
                if (!inA[r]) connectivity[r] += w;
            }
        }

        uint32_t s = reps[0], t = reps[0];
        uint64_t cutOfPhase = 0;

        for (size_t step = 1; step < reps.size(); ++step) {
            uint32_t best = 0;
            uint64_t bestConn = 0;
            bool found = false;
            for (uint32_t v : reps) {
                if (!inA[v] && (!found || connectivity[v] > bestConn)) {
                    best = v;
                    bestConn = connectivity[v];
                    found = true;
                }
            }

            s = t;
            t = best;
            cutOfPhase = bestConn;
            inA[best] = true;

            for (uint32_t ei : incident[best]) {
                if (!edgeCrossed[ei]) {
                    edgeCrossed[ei] = true;
                    uint64_t w = H.edges[ei].weight;
                    for (uint32_t r : edgeReps[ei]) {
                        if (!inA[r]) connectivity[r] += w;
                    }
                }
            }
        }

        minCut = std::min(minCut, cutOfPhase);
        dsu.unite(s, t);
    }

    return minCut;
}
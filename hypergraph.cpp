//
// Created by Mero Elmarassy on 12/8/25.
//
#include <unordered_set>
#include <algorithm>
#include <random>
#include <vector>

#include "hypergraph.h"


Hypergraph kUniformHypergraph(std::uniform_int_distribution<uint32_t> weight_dist, int n, int m, int k, int seed) {
    Hypergraph hypergraph;
    hypergraph.n = n;
    std::mt19937 rng(seed);
    std::vector<uint32_t> vertices(n);
    for (uint32_t i = 0; i < n; ++i) vertices[i] = i;

    auto generate_k_subset = [&](std::mt19937 &rng) {
        std::vector<uint32_t> subset(vertices.begin(), vertices.end());
        std::shuffle(subset.begin(), subset.end(), rng);
        subset.resize(k);
        std::sort(subset.begin(), subset.end());
        return subset;
    };

    std::unordered_set<std::string> edgeSet;
    int generated = 0;

    while (generated < m) {
        auto edgeVertices = generate_k_subset(rng);

        std::string key;
        for (auto v : edgeVertices) key += std::to_string(v) + ",";

        if (edgeSet.insert(key).second) {
            Hyperedge edge;
            edge.vertices = edgeVertices;
            edge.weight = weight_dist(rng);
            hypergraph.edges.push_back(edge);
            generated ++;
        }
    }

    return hypergraph;
}


#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <unordered_set>
#include <vector>

#ifndef HYPERGRAPH
#define HYPERGRAPH

struct Hyperedge {
    std::vector<uint32_t> vertices;
    uint32_t weight{};
    Hyperedge() = default;
    Hyperedge(std::vector<uint32_t> v, uint32_t w)
        : vertices(std::move(v)), weight(w) {}
};

struct Hypergraph {
    uint32_t n{};
    std::vector<Hyperedge> edges;

    explicit Hypergraph() = default;

    explicit Hypergraph(const std::string &fileName) {
        std::ifstream fin(fileName);
        int numHyperedges, numVertices;
        fin >> numHyperedges >> numVertices;
        edges = std::vector<Hyperedge>(numHyperedges);
        n = numVertices;
        std::string line;

        std::mt19937 gen(0);
        std::uniform_int_distribution<> distrib(1, 100);

        std::getline(fin, line);

        for (int i = 0; i < numHyperedges; i++) {
            std::getline(fin, line);
            std::istringstream iss(line);
            std::vector<uint32_t> edgeVertices;

            uint32_t vertexInd;
            while (iss >> vertexInd) {
                edgeVertices.push_back(vertexInd);
            }
            uint32_t randomWeight = distrib(gen);
            edges[i].weight = randomWeight;
            edges[i].vertices = edgeVertices;
        }
    };
};

Hypergraph
kUniformHypergraph(std::uniform_int_distribution<uint32_t> weight_dist, int n,
                   int m, int k, int seed) {
    std::cout << "generated with seed " << seed << std::endl;
    Hypergraph hypergraph;
    hypergraph.n = n;
    std::mt19937 rng(seed);
    std::vector<uint32_t> vertices(n);
    for (uint32_t i = 0; i < n; ++i)
        vertices[i] = i;

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
        for (auto v : edgeVertices)
            key += std::to_string(v) + ",";

        if (edgeSet.insert(key).second) {
            Hyperedge edge;
            edge.vertices = edgeVertices;
            edge.weight = weight_dist(rng);
            hypergraph.edges.push_back(edge);
            generated++;
        }
    }

    return hypergraph;
}

#endif // HYPERGRAPH_MIN_CUT_HYPERGRAPH_H

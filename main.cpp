#include <algorithm>
#include <cstdint>
#include <expected>
#include <iostream>
#include <random>
#include <vector>
#include <unordered_set>
#include "hypergraph.h"


double computeWeight(std::vector<Hyperedge> &edges) {
    double weight = 0.0;
    for (auto &edge: edges) {
        weight += edge.weight;
    }
    return weight;
}

int chooseRandomEdge(const Hypergraph& H, std::mt19937_64& rng) {
    if (H.edges.empty()) return -1;
    std::vector<double> weights;
    weights.reserve(H.edges.size());
    for (auto& e : H.edges) {
        weights.push_back(e.weight);
    }
    std::discrete_distribution<int> dist(weights.begin(), weights.end());
    return dist(rng);
}

double redoProbability(int n, int edges_size, int k) {
    if (k <= 1) return 0.0;
    if (edges_size >= n) return 1.0;
    if (n - edges_size < k - 1) return 1.0;

    double prod = 1.0;
    for (int i = 0; i <= k - 2; ++i) {
        prod *= (double)(n - edges_size - i) / (double)(n - i);
    }

    double z = 1.0 - prod;
    return std::clamp(z, 0.0, 1.0);
}

void getKSpanning(Hypergraph& H, int k, std::vector<Hyperedge>& S) {
    const int minimum_size = std::max(0, (int)(H.vertices.size()) - k + 2);

    std::vector<Hyperedge> kept;
    kept.reserve(H.edges.size());

    for (auto& edge : H.edges) {
        if ((int)edge.vertices.size() >= minimum_size) {
            S.push_back(edge);
        } else {
            kept.push_back(edge);
        }
    }
    H.edges = std::move(kept);
}

Hypergraph contract(const Hypergraph& H, int edgeIndex) {
    const Hyperedge& e = H.edges[edgeIndex];
    int n = (int)(H.vertices.size());

    Hypergraph contractedGraph;

    for (int vertexIndex = 0; vertexIndex < H.vertices.size(); vertexIndex++) {
        const auto vertex = &H.vertices[vertexIndex];

    }



}


std::vector<Hyperedge> BranchingContract(Hypergraph H, int k,
                                         std::vector<Hyperedge> S,
                                         std::mt19937_64& rng) {
    getKSpanning(H, k, S);

    if (H.edges.empty()) return S;

    int idx = chooseRandomEdge(H, rng);
    Hyperedge& chosen = H.edges[idx];

    double z = redoProbability(H.vertices.size(), static_cast<int>(chosen.vertices.size()), k);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double r = dist(rng);

    Hypergraph Hc = contract(H, idx);
    auto cut_contracted = BranchingContract(std::move(Hc), k, S, rng);

    if (r <= z) {
        auto cut_uncontracted = BranchingContract(H, k, S, rng);

        return (computeWeight(cut_uncontracted) < computeWeight(cut_contracted))
               ? cut_uncontracted
               : cut_contracted;
    }

    return cut_contracted;
}

std::vector<Hyperedge> BranchingContractMain(const Hypergraph& H_in, int k, uint64_t seed = 0) {
    std::mt19937_64 rng;

    if (seed == 0) {
        std::seed_seq seq { std::random_device{}(), std::random_device{} () };
        rng.seed(seq);
    } else {
        rng.seed(seed);
    }
    return BranchingContract(H_in, k, {}, rng);
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
            {1.0, {0, 1, 2}},
            {2.5, {2, 3}},
            {0.7, {1, 3, 4}}
    };

    int k = 2;
    auto result = BranchingContractMain(H, k);

    std::cout << "Cut edges (total weight = " << computeWeight(result) << "):\n";
    for (auto& edge : result) {
        std::cout << "w=" << edge.weight << " vertices:";
        for (int v : edge.vertices) std::cout << " " << v;
        std::cout << std::endl;
    }
}

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <expected>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include<fstream>
#include<sstream>
#include <unordered_set>

#include "hypergraph.h"


struct DSU {
    std::vector<int> parent;

    explicit DSU(uint32_t n) : parent(n) {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(uint32_t x) {
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    }

    void unite(uint32_t a, uint32_t b) noexcept {
        a = find(a); b = find(b);
        if (a != b) parent[b] = a;
    }
};


double redo_probability(int n, int e_size, int k) noexcept {
    if (k <= 1) return 0.0;
    if (e_size >= n) return 1.0;
    if (n - e_size < k - 1) return 1.0;
    double prod = 1.0;
    for (int i = 0; i <= k - 2; ++i) {
        prod *= (double)(n - e_size - i) / (double)(n - i);
    }
    double z = 1.0 - prod;
    return std::clamp(z, 0.0, 1.0);
}

void extract_k_spanning(Hypergraph& H, int k, std::vector<Hyperedge>& S) {
    const int threshold = std::max(0, (int) H.n - k + 2);
    std::vector<Hyperedge> kept;
    kept.reserve(H.edges.size());
    for (auto& e : H.edges) {
        if ((int)e.vertices.size() >= threshold) {
            S.push_back(e);
        } else {
            kept.push_back(e);
        }
    }
    H.edges = std::move(kept);
}

Hypergraph contract(const Hypergraph& H, int edgeIndex) {
    DSU dsu(H.n);
    const auto& edge = H.edges[edgeIndex];

    if (!edge.vertices.empty()) {
        for (size_t i = 1; i < edge.vertices.size(); ++i)
            dsu.unite(edge.vertices[0], edge.vertices[i]);
    }

    std::vector<int> repr(H.n, -1);
    int next_id = 0;

    for (int v = 0; v < H.n; ++v) {
        int r = dsu.find(v);
        if (repr[r] == -1) repr[r] = next_id++;
    }

    Hypergraph H2;
    H2.n = next_id;

    H2.edges.reserve(H.edges.size());

    for (int i = 0; i < (int)H.edges.size(); ++i) {
        if (i == edgeIndex) continue;
        const auto& e = H.edges[i];
        std::vector<uint32_t> unique;
        unique.reserve(e.vertices.size());
        for (int v : e.vertices) {
            int id = repr[dsu.find(v)];
            unique.push_back(id);
        }
        sort(unique.begin(), unique.end());
        unique.erase(std::unique(unique.begin(), unique.end()), unique.end());

        if (unique.size() > 1) {
            Hyperedge e2;
            e2.weight = e.weight;
            e2.vertices = std::move(unique);
            H2.edges.push_back(std::move(e2));
        }
    }
    return H2;
}


int select_random_edge(const Hypergraph& H, std::mt19937_64& rng) {
    if (H.edges.empty()) return -1;

    std::vector<double> weights;
    weights.reserve(H.edges.size());
    for (auto& e : H.edges) weights.push_back(e.weight);

    std::discrete_distribution<int> dist(weights.begin(), weights.end());
    return dist(rng);
}

double total_weight(const std::vector<Hyperedge>& S) noexcept {
    double s = 0.0;
    for (auto& e : S) {
        s += e.weight;
    }
    return s;
}

class DidNotFinish: std::exception {};

std::vector<Hyperedge> BranchingContract(
        Hypergraph H,
        int k,
        std::vector<Hyperedge> S,
        std::mt19937_64& rng,
        uint64_t& contractions,
        uint64_t& runtime,
        uint64_t cutoff
) {
    extract_k_spanning(H, k, S);
    if (runtime >= cutoff) {
        throw DidNotFinish();
    }

    runtime += (H.edges.size() * H.n);
    contractions ++;
    if (H.edges.empty()) return S;

    const int edgeIndex = select_random_edge(H, rng);
    const Hyperedge& chosen = H.edges[edgeIndex];

    const long double z = redo_probability(H.n, (int)chosen.vertices.size(), k);

    std::uniform_real_distribution<long double> dist(0.0L, 1.0L);
    const long double r = dist(rng);

    Hypergraph Hc = contract(H, edgeIndex);
    auto cut_contracted = BranchingContract(std::move(Hc), k, S, rng, contractions, runtime, cutoff);

    if (r <= z) {
        auto cut_uncontracted = BranchingContract(H, k, S, rng, contractions, runtime, cutoff);
        return (total_weight(cut_uncontracted) < total_weight(cut_contracted))
               ? cut_uncontracted
               : cut_contracted;
    }

    return cut_contracted;
}


struct ContractionResult {
    uint64_t numContractedEdges;
    uint64_t cutWeight;
    uint64_t totalContractions;
    uint64_t totalRuntime;
};


ContractionResult BranchingContractMain(const Hypergraph H_in, int k, uint64_t& contractions, uint64_t& runtime, uint64_t cutoff, uint64_t seed = 0) {
    std::mt19937_64 rng;
    if (seed == 0) {
        std::seed_seq seq { std::random_device{}(), std::random_device{}() };
        rng.seed(seq);
    } else {
        rng.seed(seed);
    }

    std::vector<Hyperedge> edges = BranchingContract(H_in, k, {}, rng, contractions, runtime, cutoff);
    uint32_t weight = 0;
    for (auto& edge: edges) weight += edge.weight;
    return {(uint32_t) edges.size(), weight, contractions};
}

uint64_t expectedRunningTime(uint64_t n, uint64_t m, uint64_t k) {
    if (k == 2) return m * n * n * ceil(log(n));
    return m * (uint64_t) pow(n, 2*k - 2);
}

ContractionResult Amplify(const Hypergraph H, uint64_t k, uint64_t batchScale, uint64_t numBatchesFactor, uint64_t baseSeed = 0) {

    uint64_t logN = (uint64_t) log(H.n);
    uint64_t T = expectedRunningTime((int) H.n, (int) H.edges.size(), k);
    uint64_t cutoff = 4 * batchScale * T * logN;
    uint64_t numBatches = numBatchesFactor * logN;
    uint64_t numIterationsPerBatch = batchScale * logN;
    std::cout << "Expected runtime is " << T << std::endl;
    std::cout << "Batch cutoff is " << cutoff << std::endl;
    std::cout << "Batch size is " << numIterationsPerBatch << std::endl;
    std::cout << "Running " << numBatches << " batches." << std::endl;

    uint64_t totalContractions = 0;
    uint64_t totalRuntime = 0;

    std::vector<ContractionResult> batchResults;

    for (int batchIndex = 0; batchIndex < numBatches; batchIndex ++) {
        uint64_t batchContractions = 0;
        uint64_t batchRuntime = 0;
        std::vector<ContractionResult> batchResult;
        for (int iteration = 0; iteration < numIterationsPerBatch; iteration ++) {
            try {
                batchResult.push_back(BranchingContractMain(H, k, batchContractions, batchRuntime, cutoff,
                                                            baseSeed + iteration + batchIndex * numIterationsPerBatch));
            } catch(DidNotFinish) {
                std::cout << "Batch #" << batchIndex << " did not finish; completed " << iteration << "/" << numIterationsPerBatch << " iterations." << std::endl;
                break;
            }
        }
        if (!batchResult.empty()) {
            auto optimal = batchResult[0];
            for (auto& result: batchResult) {
                if (result.cutWeight < optimal.cutWeight) {
                    optimal = result;
                }
            }
            batchResults.push_back(optimal);
            std::cout << "Batch #" << batchIndex << " completed in " << batchRuntime << " units (" << ((double)batchRuntime)/((double)(cutoff)) * 100 << "% of maximum cutoff)." << std::endl;
        } else {
            std::cout << "Batch #" << batchIndex << " did not complete any iterations." << std::endl;
        }
        totalContractions += batchContractions;
        totalRuntime += batchRuntime;
    }
    if (!batchResults.empty()) {
        auto optimal = batchResults[0];
        for (auto& result: batchResults) {
            if (result.cutWeight < optimal.cutWeight) {
                optimal = result;
            }
        }
        optimal.totalContractions = totalContractions;
        optimal.totalRuntime = totalRuntime;
        return optimal;
    } else {
        std::cout << "All batches failed to complete; did not find a minimum cut." << std::endl;
        throw DidNotFinish();
    }



}

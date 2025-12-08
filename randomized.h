//
// Created by Mero Elmarassy on 12/8/25.
//

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

#ifndef HYPERGRAPH_MIN_CUT_RANDOMIZED_H
#define HYPERGRAPH_MIN_CUT_RANDOMIZED_H



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


double redo_probability(int n, int e_size, int k);

void extract_k_spanning(Hypergraph& H, int k, std::vector<Hyperedge>& S);

Hypergraph contract(const Hypergraph& H, int idx);


int select_random_edge(const Hypergraph& H, std::mt19937_64& rng);

double total_weight(const std::vector<Hyperedge>& S);


class DidNotFinish: std::exception {};

std::vector<Hyperedge> BranchingContract(
        Hypergraph H,
        int k,
        std::vector<Hyperedge> S,
        std::mt19937_64& rng,
        uint64_t& contractions,
        uint64_t& runtime,
        uint64_t cutoff
);


struct ContractionResult {
    uint64_t numContractedEdges;
    uint64_t cutWeight;
    uint64_t totalContractions;
    uint64_t totalRuntime;
};


ContractionResult BranchingContractMain(const Hypergraph H_in, int k, uint64_t& contractions, uint64_t& runtime, uint64_t cutoff, uint64_t seed = 0);

ContractionResult Amplify(const Hypergraph H, uint64_t k, uint64_t batchScale, uint64_t numBatchesFactor, uint64_t baseSeed = 0);


#endif //HYPERGRAPH_MIN_CUT_RANDOMIZED_H

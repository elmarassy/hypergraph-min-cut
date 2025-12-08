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
    for (int edgeIndex = 0; edgeIndex < H.edges.size(); edgeIndex++) {
        auto& edge = H.edges[edgeIndex];
        if ((int) edge.vertices.size() >= minimum_size) {
            S.push_back(edge);
            for (auto& vertexIndex: edge.vertices) {
                auto& vertex = H.vertices[vertexIndex];
                vertex.incidentEdges.erase(edgeIndex);
            }
        } else {
            kept.push_back(edge);
        }
    }
    H.edges = std::move(kept);
}

Hypergraph contract(const Hypergraph& H, int edgeIndex) {
    const Hyperedge& e = H.edges[edgeIndex];
    int n = (int) H.vertices.size();
    int m = (int) H.edges.size();

    Hypergraph contractedGraph;
    auto& contractedEdge = H.edges[edgeIndex];

    std::vector<int> edgeMap; //maps from old edge indices to new edge indices
    edgeMap.reserve(m);
    for (int i = 0; i < edgeIndex; i++) {
        auto& originalEdge = H.edges[i];
        contractedGraph.edges.push_back({originalEdge.weight, {}});
        edgeMap.push_back(i);
    }
    edgeMap.push_back(-1);
    for (int i = edgeIndex+1; i < m; i++) {
        auto& originalEdge = H.edges[i];
        contractedGraph.edges.push_back({originalEdge.weight, {}});
        edgeMap.push_back(i-1);
    }

    std::vector<int> vertexMap; //maps from new vertex indices to old vertex indices
    vertexMap.reserve(n-1);
    for (int vertexIndex = 0; vertexIndex < n; vertexIndex++) {
        if (!contractedEdge.vertices.contains(vertexIndex)) {
            //the vertex is in V' (V \ e)
            vertexMap.push_back(vertexIndex);
            contractedGraph.vertices.emplace_back();
        }
    }

    std::vector<int> oldEdgeIncidenceSizes(m, -1);

    for (int newVertexIndex = 0; newVertexIndex < contractedGraph.vertices.size(); newVertexIndex++) {
        auto& newVertex = contractedGraph.vertices[newVertexIndex];
        auto& oldVertex = H.vertices[vertexMap[newVertexIndex]];

        for (auto oldEdgeIndex: oldVertex.incidentEdges) {
            auto& oldIncidentEdge = H.edges[oldEdgeIndex];
            oldEdgeIncidenceSizes[edgeMap[oldEdgeIndex]] = (int) oldIncidentEdge.vertices.size();
            auto& newIncidentEdge = contractedGraph.edges[edgeMap[oldEdgeIndex]];
            newVertex.incidentEdges.insert(edgeMap[oldEdgeIndex]);
            newIncidentEdge.vertices.insert(newVertexIndex);
        }
    }

    std::vector<Hyperedge> finalEdges;

    Vertex contractedVertex;
    int contractedVertexIndex = (int) contractedGraph.vertices.size();

    for (int i = 0; i < contractedGraph.edges.size(); i++) {
        auto& newEdge = contractedGraph.edges[i];
        if (!newEdge.vertices.empty()) {
            if (newEdge.vertices.size() < oldEdgeIncidenceSizes[i]) {
                newEdge.vertices.insert(contractedVertexIndex);
                contractedVertex.incidentEdges.insert(i);
            }
            finalEdges.push_back(std::move(newEdge));
        }
    }
    contractedGraph.vertices.push_back(contractedVertex);
    contractedGraph.edges = std::move(finalEdges);
    return contractedGraph;
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
//    Hypergraph H;
//    H.vertices = {
//            {{0}},
//            {{0, 2}},
//            {{0, 1}},
//            {{1, 2}},
//            {{2}}
//    };
//    H.edges = {
//            {50, {0, 1, 2}},
//            {3, {2, 3}},
//            {10, {1, 3, 4}}
//    };

    Hypergraph H("sat14_q_query_3_L150_coli.sat.cnf.dual.hgr");
    std::cout << H.vertices.size() << std::endl;

    int k = 5;
    auto result = BranchingContractMain(H, k, 3);

    std::cout << "Cut edges (total weight = " << computeWeight(result) << "):\n";
    for (auto& edge : result) {
        std::cout << "w=" << edge.weight << " vertices:";
        for (int v : edge.vertices) std::cout << " " << v;
        std::cout << std::endl;
    }
}

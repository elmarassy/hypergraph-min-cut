//
// Created by Mero Elmarassy on 12/7/25.
//

#include<iostream>
#include<vector>
#include<fstream>
#include<sstream>
#include<random>

#ifndef HYPERGRAPH_MIN_CUT_HYPERGRAPH_H
#define HYPERGRAPH_MIN_CUT_HYPERGRAPH_H


struct Hyperedge {
    std::vector<uint32_t> vertices;
    uint32_t weight{};
};

struct Hypergraph {
    uint32_t n {};
    std::vector<Hyperedge> edges;

    explicit Hypergraph() = default;

    explicit Hypergraph(const std::string& fileName) {
        std::ifstream fin(fileName);
        int numHyperedges, numVertices;
        fin >> numHyperedges >> numVertices;
        edges = std::vector<Hyperedge>(numHyperedges);
        n = numVertices;
        std::string line;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(1, 100);

        std::getline(fin, line);

        for (int i = 0; i < numHyperedges; i++) {
            std::getline(fin, line);
            std::istringstream iss(line);
            std::vector<uint32_t> edgeVertices;

            uint32_t vertexInd;
            while (iss >> vertexInd){
                edgeVertices.push_back(vertexInd);
            }
            uint32_t randomWeight = distrib(gen);
            edges[i].weight = randomWeight;
            edges[i].vertices = edgeVertices;
        }
    };
};

Hypergraph kUniformHypergraph(std::uniform_int_distribution<uint32_t> weight_dist, int n, int m, int k, int seed);

#endif //HYPERGRAPH_MIN_CUT_HYPERGRAPH_H

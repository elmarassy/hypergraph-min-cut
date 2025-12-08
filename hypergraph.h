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
    int weight{};
    std::unordered_set<int> vertices;

    std::string toString() const {
        std::ostringstream oss;
        oss << "Edge of weight " << weight << ", vertices: ";
        for (int v : vertices)
            oss << v << " ";
        oss << "\n";
        return oss.str();
    }
};

struct Vertex {
    std::unordered_set<int> incidentEdges;

    std::string toString() const {
        std::ostringstream oss;
        oss << "Vertex with hyperedges:";
        for (int e : incidentEdges)
            oss << e << " ";
        oss << "\n";
        return oss.str();
    }
};

struct Hypergraph {
    std::vector<Vertex> vertices;
    std::vector<Hyperedge> edges;

    Hypergraph() = default;

    explicit Hypergraph(const std::string& fileName) {
        std::ifstream fin(fileName);

        int numHyperedges, numVertices;
        fin >> numHyperedges >> numVertices;

        edges = std::vector<Hyperedge>(numHyperedges);
        vertices = std::vector<Vertex>(numVertices);
        std::string line;

        // Generate the Randomness
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(1, 100);

        std::getline(fin, line);

        for (int i = 0; i < numHyperedges; i++) {
            std::getline(fin, line);
            std::istringstream iss(line);
            std::unordered_set<int> edgeVertices;

            // Assigning the Edges in the Graph
            int vertexInd;
            while (iss >> vertexInd){
                edgeVertices.insert(vertexInd-1);

                // Assigning the Vertices in the Graph
                vertices[vertexInd-1].incidentEdges.insert(i);
            }
            int randomWeight = distrib(gen);
            edges[i].weight = randomWeight;
            edges[i].vertices = edgeVertices;
        }
    };
};



#endif //HYPERGRAPH_MIN_CUT_HYPERGRAPH_H

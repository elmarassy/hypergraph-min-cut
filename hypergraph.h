//
// Created by Mero Elmarassy on 12/7/25.
//

#include<iostream>
#include<vector>

#ifndef HYPERGRAPH_MIN_CUT_HYPERGRAPH_H
#define HYPERGRAPH_MIN_CUT_HYPERGRAPH_H


struct Hyperedge {
    double weight{};
    std::vector<int> vertices;
};

struct Vertex {
    std::vector<int> incidentEdges;
};

struct Hypergraph {
    std::vector<Vertex> vertices;
    std::vector<Hyperedge> edges;
};



#endif //HYPERGRAPH_MIN_CUT_HYPERGRAPH_H

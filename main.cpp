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
#include "randomized.h"
#include <chrono>
#include <filesystem>
//#include "deterministichypergraph.h"
#include "deterministicbenchmark.h"

void experimentOneRandomized(){
    std::ofstream outputFile("outputs/exp1Randomized.txt");
    outputFile << "Format: n k #contractions #weight edges-cut" << std::endl;
    std::vector<int> sizes = {10, 50, 100, 500, 1000};
    for (int size: sizes){
        outputFile << size << " 5 ";
        Hypergraph H = kUniformHypergraph(std::uniform_int_distribution<uint32_t>(1, 100), size, size, 5, 1);
        auto result = Amplify(H, 2, 2, 1);
        outputFile << result.totalContractions << " " << result.cutWeight << " " << result.numContractedEdges << std::endl;
    }
    outputFile << std::endl;
    for (int size: sizes){
        int k = (int)size/2;
        outputFile << size << " " << k << " ";
        Hypergraph H = kUniformHypergraph(std::uniform_int_distribution<uint32_t>(1, 100), size, size, k, 1);
        auto result = Amplify(H, 2, 2, 1);
        outputFile << result.totalContractions << " " << result.cutWeight << " " << result.numContractedEdges << std::endl;
    }
    outputFile.close();
}

void experimentTwoRandomized(){
    std::string path_to_dir = "/Users/rushil/Documents/MIT/Fall25/6.5220/hypergraph-min-cut/circuit_hypergraphs";
    std::ofstream outputFile("outputs/exp2Randomized.txt");
    outputFile << "file_name #contractions #weight edges-cut time" << std::endl;
    for (const auto& entry: std::filesystem::directory_iterator(path_to_dir)){
        std::string file_name = entry.path().filename();
        // std::string file_name = "c17.mtx.hgr";
        std::string full_file_name = path_to_dir+"/"+file_name;
        outputFile << file_name << " ";
        Hypergraph H{full_file_name};
        auto result = Amplify(H, 2, 2, 1);
        outputFile << result.totalContractions << " " << result.cutWeight << " " << result.numContractedEdges << std::endl;
    }
    outputFile.close();
}

void experimentTwoDeterministic(){
    std::string path_to_dir = "/Users/rushil/Documents/MIT/Fall25/6.5220/hypergraph-min-cut/circuit_hypergraphs";
    std::ofstream outputFile("outputs/exp2Deterministic.txt");
    outputFile << "file_name #weight time" << std::endl;
    for (const auto& entry: std::filesystem::directory_iterator(path_to_dir)){
        std::string file_name = entry.path().filename();
        std::cout << "Running " << file_name << std::endl;
        std::string full_file_name = path_to_dir+"/"+file_name;
        outputFile << file_name << " ";
        Hypergraph H{full_file_name};
        auto cutWeight = deterministicMinCut(H);
        outputFile << cutWeight << std::endl;
    }
    outputFile.close();
}

int main() {
    // Hypergraph H;
    // H.edges = {
    //         {{0, 1, 2, 7}, 50},
    //         {{2, 3, 7}, 3},
    //         {{1, 3, 4, 6}, 10},
    //         {{0, 2, 4, 6}, 5},
    //         {{3, 5, 7}, 7},
    //         {{0, 6}, 1}
    // };
    // H.n = 8;
    // auto cutWeight = deterministicMinCut(H);
    // std::cout << cutWeight << std::endl;

    experimentTwoDeterministic();
}

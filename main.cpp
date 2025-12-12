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
#include "deterministic.h"

void experimentOneRandomized(){
    std::ofstream outputFile("outputs/exp1Randomized.txt");
    outputFile << "n m k #contractions #weight run-time" << std::endl;
    for (int n = 100; n <= 1000; n += 100){
        for (int m = 100; m <= 1000; m += 100){
            outputFile << n << " " << m << " 10 ";
            Hypergraph H = kUniformHypergraph(std::uniform_int_distribution<uint32_t>(1, 100), n, m, 10, 1);
            auto start = std::chrono::high_resolution_clock::now();
            auto result = randomizedMinKCut(H, 2, 2, 1, 1);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            outputFile << result.totalContractions << " " << result.cutWeight << " " << result.totalRuntime << " " << duration.count() << std::endl;
        }
    }
    outputFile << std::endl;
    for (int n = 100; n <= 1000; n += 100){
        for (int m = 100; m <= 1000; m += 100){
            int k = (int)std::log(n);
            outputFile << n << " " << m << " " << k << " ";
            Hypergraph H = kUniformHypergraph(std::uniform_int_distribution<uint32_t>(1, 100), n, m, k, 1);
            auto start = std::chrono::high_resolution_clock::now();
            auto result = randomizedMinKCut(H, 2, 2, 1, 1);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            outputFile << result.totalContractions << " " << result.cutWeight << " " << result.totalRuntime << " " << duration.count() << std::endl;
        }
    }
    outputFile.close();
}

void experimentTwoRandomized(){
    std::string path_to_circuit = "/Users/rushil/Documents/MIT/Fall25/6.5220/hypergraph-min-cut/circuit_hypergraphs";
    std::ofstream outputFile("outputs/exp2Randomized.txt");
    outputFile << "file_name #contractions #weight edges-cut time" << std::endl;
    for (const auto& entry: std::filesystem::directory_iterator(path_to_circuit)){
        std::string file_name = entry.path().filename();
        std::string full_file_name = path_to_circuit+"/"+file_name;
        outputFile << file_name << " ";
        Hypergraph H{full_file_name};
        auto start = std::chrono::high_resolution_clock::now();
        auto result = randomizedMinKCut(H, 2, 2, 1, 1);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        outputFile << result.totalContractions << " " << result.cutWeight << " " << result.totalRuntime << " " << duration.count() << std::endl;
    }
    outputFile << std::endl;
    std::string path_to_hyperff = "/Users/rushil/Documents/MIT/Fall25/6.5220/hypergraph-min-cut/hyperff_hypergraphs";
    for (const auto& entry: std::filesystem::directory_iterator(path_to_hyperff)){
        std::string file_name = entry.path().filename();
        std::cout << "Running " << file_name << std::endl;
        std::string full_file_name = path_to_hyperff+"/"+file_name;
        outputFile << file_name << " ";
        Hypergraph H{full_file_name};
        auto start = std::chrono::high_resolution_clock::now();
        auto result = randomizedMinKCut(H, 2, 2, 1, 1);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        outputFile << result.totalContractions << " " << result.cutWeight << " " << result.totalRuntime << " " << duration.count() << std::endl;
    }

    outputFile.close();
}

void experimentOneDeterministic(){
    std::ofstream outputFile("outputs/exp1Deterministic.txt");
    outputFile << "n m k weight run-time" << std::endl;
    std::vector<int> sizes = {1000};
    for (int n = 100; n <= 1000; n += 100){
        for (int m = 100; m <= 1000; m += 100){
            outputFile << n << " " << m << " 10 ";
            Hypergraph H = kUniformHypergraph(std::uniform_int_distribution<uint32_t>(1, 100), n, m, 10, 1);
            auto start = std::chrono::high_resolution_clock::now();
            auto cutWeight = deterministicMinCut(H);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            outputFile << cutWeight.cutWeight << " " << duration.count() << std::endl;
        }
    }
    outputFile << std::endl;
    for (int n = 100; n <= 1000; n += 100){
        for (int m = 100; m <= 1000; m += 100){
            int k = (int)log(n);
            outputFile << n << " " << m << " " << k << " ";
            Hypergraph H = kUniformHypergraph(std::uniform_int_distribution<uint32_t>(1, 100), n, m, k, 1);
            auto start = std::chrono::high_resolution_clock::now();
            auto cutWeight = deterministicMinCut(H);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            outputFile << cutWeight.cutWeight << " " << duration.count() << std::endl;
        }
    }
}

void experimentTwoDeterministic(){
    std::string path_to_circuit = "/Users/rushil/Documents/MIT/Fall25/6.5220/hypergraph-min-cut/circuit_hypergraphs";
    std::ofstream outputFile("outputs/exp2Deterministic.txt");
    outputFile << "file_name #weight time" << std::endl;
    for (const auto& entry: std::filesystem::directory_iterator(path_to_circuit)){
        std::string file_name = entry.path().filename();
        std::cout << "Running " << file_name << std::endl;
        std::string full_file_name = path_to_circuit+"/"+file_name;
        outputFile << file_name << " ";
        Hypergraph H{full_file_name};
        auto start = std::chrono::high_resolution_clock::now();
        auto result = deterministicMinCut(H);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        outputFile << result.cutWeight << " " << duration.count() << std::endl;
    }
    outputFile << std::endl;
    std::string path_to_hyperff = "/Users/rushil/Documents/MIT/Fall25/6.5220/hypergraph-min-cut/hyperff_hypergraphs";
    for (const auto& entry: std::filesystem::directory_iterator(path_to_hyperff)){
        std::string file_name = entry.path().filename();
        std::cout << "Running " << file_name << std::endl;
        std::string full_file_name = path_to_hyperff+"/"+file_name;
        outputFile << file_name << " ";
        Hypergraph H{full_file_name};
        auto start = std::chrono::high_resolution_clock::now();
        auto result = deterministicMinCut(H);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        outputFile << result.cutWeight << " " << duration.count() << std::endl;
    }

    outputFile.close();
}

int main() {
    experimentOneRandomized();
}

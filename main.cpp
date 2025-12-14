#include "deterministic.h"
#include "randomized.h"
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>


void experimentOne(std::vector<size_t> nValues, std::vector<size_t> mValues, size_t repetitions, size_t seedMult) {
    std::ofstream randConst(std::format("outputs/exp1_randomized_const_{}.txt", seedMult));
    std::ofstream randLog(std::format("outputs/exp1_randomized_sqrt_{}.txt", seedMult));

    std::ofstream detConst(std::format("outputs/exp1_deterministic_const_{}.txt", seedMult));
    std::ofstream detLog(std::format("outputs/exp1_deterministic_sqrt_{}.txt", seedMult));
    int t = 154;
    int baseSeed = seedMult * 1000000;
    for (auto n : nValues) {
        for (auto m : mValues) {
            Hypergraph constK = kUniformHypergraph(std::uniform_int_distribution<uint32_t>(1, 100), n, m, 10, t + baseSeed);
            Hypergraph logK = kUniformHypergraph(std::uniform_int_distribution<uint32_t>(1, 100), n, m, (int)std::sqrt(n), t + baseSeed);
            detConst << n << " " << m << std::endl;
            detLog << n << " " << m << std::endl;
            randConst << n << " " << m << std::endl;
            randLog << n << " " << m << std::endl;

            for (int i = 0; i < repetitions; i++) {

                auto startRandConst = std::chrono::high_resolution_clock::now();
                auto resultRandConst = randomizedMinKCut(constK, 2, 2, 1, i + 1 + baseSeed);
                auto endRandConst = std::chrono::high_resolution_clock::now();
                auto durationRandConst = std::chrono::duration_cast<std::chrono::microseconds>(endRandConst - startRandConst);
                randConst << resultRandConst.totalRuntime << " " << resultRandConst.totalContractions << " " << resultRandConst.cutWeight << " " << durationRandConst.count() << std::endl;
                std::cout << "rand const " << resultRandConst.cutWeight << std::endl;
                auto startRandLog = std::chrono::high_resolution_clock::now();
                auto resultRandLog = randomizedMinKCut(logK, 2, 2, 1, i + 1 + baseSeed);
                auto endRandLog = std::chrono::high_resolution_clock::now();
                auto durationRandLog = std::chrono::duration_cast<std::chrono::microseconds>(endRandLog - startRandLog);
                randLog << resultRandLog.totalRuntime << " " << resultRandLog.totalContractions << " " << resultRandLog.cutWeight << " " << durationRandLog.count() << std::endl;
                std::cout << "rand log " << resultRandLog.cutWeight << std::endl;

                auto startDetConst = std::chrono::high_resolution_clock::now();
                auto resultDetConst = deterministicMinCut(constK);
                auto endDetConst = std::chrono::high_resolution_clock::now();
                auto durationDetConst = std::chrono::duration_cast<std::chrono::microseconds>(endDetConst - startDetConst);
                detConst << resultDetConst << " " << durationDetConst.count() << std::endl;
                std::cout << "det const" << resultDetConst << std::endl;

                auto startDetLog = std::chrono::high_resolution_clock::now();
                auto resultDetLog = deterministicMinCut(logK);
                auto endDetLog = std::chrono::high_resolution_clock::now();
                auto durationDetLog = std::chrono::duration_cast<std::chrono::microseconds>(endDetLog - startDetLog);
                detLog << resultDetLog << " " << durationDetLog.count() << std::endl;
                std::cout << "det log" << resultDetLog << std::endl;

            }
            std::cout << "Completed graphs (" << n << ", " << m << ")" << std::endl;
            t += 1;
        }
    }
    randConst.close();
    randLog.close();
    detConst.close();
    detLog.close();
}


void experimentTwoRandomized() {
    std::string path_to_circuit = "./circuit_hypergraphs";
    std::ofstream outputFile("outputs/exp2Randomized.txt");
    outputFile << "file_name trial #contractions #weight edges-cut time" << std::endl;
    for (const auto &entry : std::filesystem::directory_iterator(path_to_circuit)) {
        for (int t = 0; t < 10; t++) {
            std::string file_name = entry.path().filename();
            std::string full_file_name = path_to_circuit + "/" + file_name;
            outputFile << file_name << " " << t << " ";
            Hypergraph H{full_file_name};
            auto start = std::chrono::high_resolution_clock::now();
            auto result = randomizedMinKCut(H, 2, 2, 1, t + 1);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            outputFile << result.totalContractions << " " << result.cutWeight << " " << result.totalRuntime << " " << duration.count() << std::endl;
        }
    }
    outputFile << std::endl;
    std::string path_to_hyperff = "./hyperff_hypergraphs";
    for (const auto &entry : std::filesystem::directory_iterator(path_to_hyperff)) {
        for (int t = 0; t < 10; t++) {
            std::string file_name = entry.path().filename();
            std::cout << "Running " << file_name << std::endl;
            std::string full_file_name = path_to_hyperff + "/" + file_name;
            outputFile << file_name << " ";
            Hypergraph H{full_file_name};
            auto start = std::chrono::high_resolution_clock::now();
            auto result = randomizedMinKCut(H, 2, 2, 1, t + 1);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            outputFile << result.totalContractions << " " << result.cutWeight << " " << result.totalRuntime << " " << duration.count() << std::endl;
        }
    }

    outputFile.close();
}


void experimentTwoDeterministic() {
    std::string path_to_circuit = "./circuit_hypergraphs";
    std::ofstream outputFile("outputs/exp2Deterministic.txt");
    outputFile << "file_name trial #weight time" << std::endl;
    for (const auto &entry : std::filesystem::directory_iterator(path_to_circuit)) {
        for (int t = 0; t < 10; t++) {
            std::string file_name = entry.path().filename();
            std::cout << "Running " << file_name << std::endl;
            std::string full_file_name = path_to_circuit + "/" + file_name;
            outputFile << file_name << " " << t << " ";
            Hypergraph H{full_file_name};
            auto start = std::chrono::high_resolution_clock::now();
            auto result = deterministicMinCut(H);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            outputFile << result << " " << duration.count() << std::endl;
        }
    }

    outputFile << std::endl;
    std::string path_to_hyperff = "./hyperff_hypergraphs";
    for (const auto &entry : std::filesystem::directory_iterator(path_to_hyperff)) {
        for (int t = 0; t < 10; t++) {
            std::string file_name = entry.path().filename();
            std::cout << "Running " << file_name << std::endl;
            std::string full_file_name = path_to_hyperff + "/" + file_name;
            outputFile << file_name << " " << t << " ";
            Hypergraph H{full_file_name};
            auto start = std::chrono::high_resolution_clock::now();
            auto result = deterministicMinCut(H);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            outputFile << result << " " << duration.count() << std::endl;
        }
    }

    outputFile.close();
}

int main(int argc, char *argv[]) {
    std::vector<size_t> nValues;
    std::vector<size_t> mValues;
    for (int i = 100; i < 1600; i += 100)
        nValues.push_back(i);
    for (int i = 100; i < 1600; i += 100)
        mValues.push_back(i);
    experimentOne(nValues, mValues, 5, 0);
    experimentTwoRandomized();
    experimentTwoDeterministic();
}

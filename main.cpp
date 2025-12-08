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


int main() {
    Hypergraph H = kUniformHypergraph(std::uniform_int_distribution<uint32_t>(1, 100), 100, 100, 54, 1);

    int k = 2;
    auto result = Amplify(H, k, 2, 1);
    std::cout << "total contractions: " << result.totalContractions << std::endl;
    std::cout << "total weight = " << result.cutWeight << std::endl;
    std::cout << "number of edges cut = " << result.numContractedEdges << std::endl;

}

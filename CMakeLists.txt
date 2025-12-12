cmake_minimum_required(VERSION 3.28)

project(hypergraph_min_cut)

set(CMAKE_CXX_STANDARD 26)

add_executable(hypergraph_min_cut
        main.cpp
        hypergraph.h
        deterministic.h
        randomized.h
)

target_compile_options(hypergraph_min_cut PRIVATE -O3 -march=native )

cmake_minimum_required(VERSION 3.28)
project(hypergraph_min_cut)

set(CMAKE_CXX_STANDARD 26)

add_executable(hypergraph_min_cut main.cpp
        hypergraph.h)

cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_COMPILER g++ CACHE STRING "C++ compiler" FORCE)
project(ccq_qdags)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3 -fopenmp")

include_directories(includes)
include_directories(external/sdsl-lite/include)
include_directories(external/sdsl-lite/build/external/libdivsufsort/include)
include_directories(src)

add_executable(j4_ghd queries/j4_ghd.cpp)

target_link_libraries(j4_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/lib/libsdsl.a)
target_link_libraries(j4_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort.a)
target_link_libraries(j4_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort64.a)


add_executable(j3_ghd queries/j3_ghd.cpp)

target_link_libraries(j3_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/lib/libsdsl.a)
target_link_libraries(j3_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort.a)
target_link_libraries(j3_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort64.a)

add_executable(ti4_ghd queries/ti4_ghd.cpp)

target_link_libraries(ti4_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/lib/libsdsl.a)
target_link_libraries(ti4_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort.a)
target_link_libraries(ti4_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort64.a)
target_link_libraries(j3_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort64.a)

add_executable(triangle_tadpole queries/triangle_tadpole.cpp)

target_link_libraries(triangle_tadpole ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/lib/libsdsl.a)
target_link_libraries(triangle_tadpole ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort.a)
target_link_libraries(triangle_tadpole ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort64.a)
target_link_libraries(j3_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort64.a)

add_executable(t4_ghd queries/t4_ghd.cpp)

target_link_libraries(t4_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/lib/libsdsl.a)
target_link_libraries(t4_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort.a)
target_link_libraries(t4_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort64.a)

add_executable(bowtie queries/bowtie.cpp
        src/joins.cpp
        includes/ghd_solver.hpp
        includes/sys/resource.h
        queries/square_barbell.cpp
        queries/square_barbell_fast.cpp)

target_link_libraries(bowtie ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/lib/libsdsl.a)
target_link_libraries(bowtie ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort.a)
target_link_libraries(bowtie ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort64.a)

add_executable(bowtie_fast queries/bowtie_fast.cpp
        src/joins.cpp
        includes/ghd_solver.hpp)

target_link_libraries(bowtie_fast ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/lib/libsdsl.a)
target_link_libraries(bowtie_fast ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort.a)
target_link_libraries(bowtie_fast ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort64.a)

add_executable(tree_decomp_brute
        src/tree_decomp_brute/tree_decomp_brute.cpp
        src/hypergraphs/hypertree_check.cpp
        src/fractional_edge_cover/fractional_edge_cover_solver.cpp)

add_executable(t3_ghd queries/t3_ghd.cpp)

target_link_libraries(t3_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/lib/libsdsl.a)
target_link_libraries(t3_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort.a)
target_link_libraries(t3_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort64.a)

add_executable(ti3_ghd queries/ti3_ghd.cpp)

target_link_libraries(ti3_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/lib/libsdsl.a)
target_link_libraries(ti3_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort.a)
target_link_libraries(ti3_ghd ${CMAKE_SOURCE_DIR}/external/sdsl-lite/build/external/libdivsufsort/lib/libdivsufsort64.a)

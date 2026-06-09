#include <bits/stdc++.h>
#include <iostream>
#include "project/fractional_edge_cover_solver/fractional_edge_cover_solver.hpp"


void triangle_test(){
    int n = 3;
    int m = 3;
    solver::Edge e1, e2, e3;
    e1.u = 0; e1.v = 1; e1.w = 1;
    e2.u = 1; e2.v = 2; e2.w = 1;
    e3.u = 2; e3.v = 0; e3.w = 1;
    std::vector<solver::Edge> edges;
    edges.emplace_back(e1);
    edges.emplace_back(e2);
    edges.emplace_back(e3);
    solver::FractionalEdgeCoverSolver fecs;
    solver::Result res = fecs.solve(edges, n);
    
    std::cout << "Test: triangle graph" << '\n';
    std::cout << "Expected edge cover: " << 1.5 << '\n';
    res.print();
}

void isolated_vertex_graph_test(){
    int n = 8;
    int m = 1;
    solver::Edge e1;
    e1.u = 0; e1.v = 6; e1.w = 1;
    std::vector<solver::Edge> edges;
    edges.emplace_back(e1);
    solver::FractionalEdgeCoverSolver fecs;
    solver::Result res = fecs.solve(edges, n);
    
    std::cout << "Test: isolated vertex" << '\n';
    std::cout << "Expected edge cover: " << "inf" << '\n';
    res.print();
}

void edge_test(){
    int n = 2;
    int m = 1;
    solver::Edge e1;
    e1.u = 0;
    e1.v = 1;
    e1.w = 1;
    std::vector<solver::Edge> edges;
    edges.emplace_back(e1);
    solver::FractionalEdgeCoverSolver fecs;
    solver::Result res = fecs.solve(edges, n);
    std::cout << "Test: graph with one edge" << '\n';
    std::cout << "Expected edge cover: " << 1.0 << '\n';
    res.print();
}

int main(){
    triangle_test();
    edge_test();
    isolated_vertex_graph_test();
    return 0;    
}
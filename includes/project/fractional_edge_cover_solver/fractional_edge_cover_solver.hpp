#pragma once
#include <vector>
#include <iostream>

namespace solver {

struct Edge {
    int u, v;
    double w;
};

// Result returned by the solver
struct Result {
    double objective_value;
    std::vector<double> solution;

    void print() {
        std::cout << "objective_value: " << objective_value << '\n';
        std::cout << "solution: " << '\n';
        for(int i = 0; i < (int)solution.size(); i++){
            std::cout << "x_" << i << ": " << solution[i] << '\n';
        }
    }
};

/*
    Solver for instances of Fractional Edge Cover on graphs.
    This solver models the problem as a linear program and solves it using the simplex method.
*/
class FractionalEdgeCoverSolver {
public:
    /*
        Solves the fractional edge cover problem for a graph G given its adjacency list adj.
        G must be undirected.

        @param edges Edge list of the graph
        @param n Number of nodes in the graph
        @return Optimization result
    */
    Result solve(std::vector<Edge>& edges, int n);
};

}
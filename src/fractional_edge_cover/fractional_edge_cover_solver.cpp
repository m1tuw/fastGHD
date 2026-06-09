#include "project/fractional_edge_cover_solver/fractional_edge_cover_solver.hpp"
#include "project/fractional_edge_cover_solver/simplex.hpp"

#include <vector>
#include <utility>
#include <cmath>

namespace solver {

Result FractionalEdgeCoverSolver::solve(std::vector<Edge>& edges, int n) {
    int m = edges.size();
    
    // LP construction
    // n + m constraints: one for each vertex, one for each edge (x_i <= 1. x_i >= 0 is automatically enforced by the solver)
    // constraints from 0 to n-1 will represent vertices. 
    // constraints from n to n+m will represent constraints on edges.
    // formally, if i >= n, i-th constraint will affect the (i-n)th edge.
    // !!WARNING!! isolated vertices will be ignored.
    std::vector<bool> isolated(n, 1);
    std::vector<std::vector<double>> A(n + m, std::vector<double>(m, 0.0));
    std::vector<double> b(n + m, -1.0);
    std::vector<double> costs(m);
    for(int i = 0; i < m; i++) {
        const Edge& e = edges[i];
        int u = e.u;
        int v = e.v;
        isolated[u] = 0;
        isolated[v] = 0;
        A[u][i] = -1.0;
        A[v][i] = -1.0;
        A[n+i][i] = 1.0;
        b[n+i] = 1.0;
    }

    for(int i = 0; i < m; i++) {
        // negate costs because this simplex implementation maximizes
        costs[i] = -edges[i].w;
    }

    for(int i = 0; i < n; i++) {
        if(isolated[i]) b[i] = 0;
    }
    
    simplex::LPSolver lp(A, b, costs);
    simplex::vd x;
    double opt = lp.solve(x);

    Result res;
    res.objective_value = -opt;
    res.solution = x;

    return res;
}

}
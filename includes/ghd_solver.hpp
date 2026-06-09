#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#include "project/fractional_edge_cover_solver/fractional_edge_cover_solver.hpp"
#include "project/hypergraphs/hypertree_check.hpp"

class GHDSolver {
public:
    struct GHDResult {
        std::vector<std::vector<int>> bags;
        std::vector<std::vector<int>> join_tree;
        double weight;
    };

    bool verbose = false;

    /*
     * List of edges must be 0-indexed.
     */
    GHDResult solve(int number_of_nodes, int number_of_edges, const std::vector<std::pair<int, int>>& edges) {
        n = number_of_nodes;
        m = number_of_edges;

        if (n < 0 || n > 20) {
            throw std::runtime_error("GHDSolver only supports 0..20 nodes");
        }

        best = std::numeric_limits<double>::infinity();
        solution.clear();
        last_opt = 0;
        best_last_opt = 0;
        prune = true;

        const std::size_t bag_count = std::size_t(1) << n;
        fec_precalc.assign(bag_count, 0.0);
        ready.assign(bag_count, 0);

        generate_permutation_of_bags();

        std::vector<bool> used_nodes(n);
        std::vector<bool> used_edges(m);
        std::vector<bool> used_bags(1 << n);
        std::vector<std::vector<int>> bags;

        if (verbose) {
            std::cout << "finding upper bound" << '\n';
        }

        prune = true;
        backtrack(used_nodes, used_edges, used_bags, 0.0, bags, edges, -1);

        if (verbose) {
            std::cout << "upper bound: " << best << '\n';
        }

        prune = false;
        if (verbose) {
            std::cout << "best_last_opt: " << best_last_opt << '\n';
        }

        best_last_opt = 0;
        backtrack(used_nodes, used_edges, used_bags, 0.0, bags, edges, best_last_opt - 1);

        GHDResult result;
        result.bags = solution;
        result.weight = best;
        result.join_tree = hypergraph::recover_join_tree(solution, n);
        return result;
    }

private:
    int n = 0;
    int m = 0;
    double best = std::numeric_limits<double>::infinity();
    std::vector<int> mask_permutation;
    bool prune = true;
    std::vector<std::vector<int>> solution;
    std::vector<double> fec_precalc;
    std::vector<unsigned char> ready;
    int last_opt = 0;
    int best_last_opt = 0;

    static int popcount_int(int value) {
        int count = 0;
        while (value != 0) {
            value &= (value - 1);
            ++count;
        }
        return count;
    }

    void generate_permutation_of_bags() {
        mask_permutation.clear();
        for (int i = 1; i < (1 << n); ++i) {
            mask_permutation.push_back(i);
        }

        std::sort(mask_permutation.begin(), mask_permutation.end(), [&](int a, int b) {
            return popcount_int(a) > popcount_int(b);
        });
    }

    void backtrack(std::vector<bool>& used_nodes,
                   std::vector<bool>& used_edges,
                   std::vector<bool>& used_bags,
                   double current_weight,
                   std::vector<std::vector<int>>& bags,
                   const std::vector<std::pair<int, int>>& edges,
                   int last) {
        int cntn = 0;
        int cntm = 0;

        for (int i = 0; i < n; ++i) {
            cntn += used_nodes[i];
        }
        for (int i = 0; i < m; ++i) {
            cntm += used_edges[i];
        }

        if (cntn == n && cntm == m) {
            if (hypergraph::is_hypertree(bags, n)) {
                if (current_weight < best) {
                    solution = bags;
                    best = current_weight;
                    if (verbose) {
                        std::cout << "new best found: " << current_weight << std::endl;
                    }
                    best_last_opt = last_opt;
                }
            }
            return;
        }

        for (int mm = last + 1; mm < static_cast<int>(mask_permutation.size()); ++mm) {
            if (bags.empty()) {
                last_opt = mm;
            }

            const int mask = mask_permutation[mm];
            if (used_bags[mask]) {
                continue;
            }

            std::vector<int> added;
            std::vector<int> removed_subsets;
            std::vector<int> added_edges;
            std::vector<solver::Edge> induced_subgraph;
            std::vector<int> isolated(n, 1);

            for (int i = 0; i < m; ++i) {
                int u = edges[i].first;
                int v = edges[i].second;
                if ((mask & (1 << u)) && (mask & (1 << v))) {
                    isolated[u] = 0;
                    isolated[v] = 0;
                    if (!used_edges[i]) {
                        added_edges.push_back(i);
                    }
                    solver::Edge e{};
                    e.u = u;
                    e.v = v;
                    e.w = 1;
                    induced_subgraph.push_back(e);
                }
            }

            std::vector<int> new_bag;
            bool bad = false;
            for (int i = 0; i < n; ++i) {
                if (mask & (1 << i)) {
                    new_bag.push_back(i);
                    if (isolated[i]) {
                        bad = true;
                    }
                }
            }

            const int d = static_cast<int>(new_bag.size());
            if (induced_subgraph.empty() || added_edges.empty() || bad) {
                continue;
            }

            double bag_cost = pow(2.0, d);
            const double M = 10.0;
            if (!ready[mask]) {
                solver::FractionalEdgeCoverSolver fecs;
                solver::Result res = fecs.solve(induced_subgraph, n);
                fec_precalc[mask] = res.objective_value;
                ready[mask] = 1;
                bag_cost *= pow(M, res.objective_value);
            } else {
                bag_cost *= pow(M, fec_precalc[mask]);
            }

            if (current_weight + bag_cost >= best) {
                continue;
            }

            for (int i = 0; i < n; ++i) {
                if (!used_nodes[i] && (mask & (1 << i))) {
                    added.push_back(i);
                }
            }

            for (int s = mask; ; s = (s - 1) & mask) {
                if (used_bags[s] == 0) {
                    removed_subsets.push_back(s);
                }
                if (s == 0) {
                    break;
                }
            }

            for (int x : added) {
                used_nodes[x] = true;
            }
            for (int s : removed_subsets) {
                used_bags[s] = true;
            }
            for (int e : added_edges) {
                used_edges[e] = true;
            }

            bags.push_back(new_bag);
            if (prune) {
                if (hypergraph::is_hypertree(bags, n)) {
                    backtrack(used_nodes, used_edges, used_bags, current_weight + bag_cost, bags, edges, mm);
                }
            } else {
                backtrack(used_nodes, used_edges, used_bags, current_weight + bag_cost, bags, edges, mm);
            }
            bags.pop_back();

            for (int x : added) {
                used_nodes[x] = false;
            }
            for (int s : removed_subsets) {
                used_bags[s] = false;
            }
            for (int e : added_edges) {
                used_edges[e] = false;
            }
        }
    }
};


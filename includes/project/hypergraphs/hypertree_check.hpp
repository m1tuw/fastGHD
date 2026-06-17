#pragma once

#include <vector>

namespace hypergraph {

// cover[v] = list of hyperedges containing vertex v
// hyperedges are indexed from 0 to m-1
// Returns true iff the hypergraph is alpha-acyclic (GYO-reducible).
bool is_hypertree(const std::vector<std::vector<int>>& bags, int n);

std::vector<std::vector<int>> recover_join_tree(const std::vector<std::vector<int>>& bags, int n);

} // namespace hypergraph
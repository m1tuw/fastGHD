#include "project/hypergraphs/hypertree_check.hpp"
#include "project/fractional_edge_cover_solver/fractional_edge_cover_solver.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <limits>

namespace hypergraph {

// bags[e] = list of original-graph vertices in bag e
// vertices are assumed to be indexed from 0 to n-1
//
// Returns true iff the bags admit a join tree, i.e. iff for every
// vertex v, the set of bags containing v forms a connected subtree.
// Equivalently, this checks alpha-acyclicity of the hypergraph whose
// hyperedges are the bags.
bool is_hypertree(const std::vector<std::vector<int>>& bags, int n) {
    if (n < 0) return false;

    const int m = static_cast<int>(bags.size());

    // Normalize bags:
    // - validate vertex ids
    // - remove duplicates inside each bag
    std::vector<std::vector<int>> verts = bags;
    for (auto& bag : verts) {
        for (int v : bag) {
            if (v < 0 || v >= n) return false;
        }
        std::sort(bag.begin(), bag.end());
        bag.erase(std::unique(bag.begin(), bag.end()), bag.end());
    }

    // Build cover[v] = list of bags containing vertex v
    std::vector<std::vector<int>> cover(n);
    for (int e = 0; e < m; ++e) {
        for (int v : verts[e]) {
            cover[v].push_back(e);
        }
    }

    std::vector<int> degV(n), szE(m);
    std::vector<unsigned char> deadV(n, 0), deadE(m, 0), inQE(m, 0);

    for (int v = 0; v < n; ++v) degV[v] = static_cast<int>(cover[v].size());
    for (int e = 0; e < m; ++e) szE[e] = static_cast<int>(verts[e].size());

    std::queue<int> qv, qe;

    auto push_vertex = [&](int v) {
        if (!deadV[v] && degV[v] <= 1) {
            qv.push(v);
        }
    };

    auto push_edge = [&](int e) {
        if (!deadE[e] && !inQE[e]) {
            inQE[e] = 1;
            qe.push(e);
        }
    };

    for (int v = 0; v < n; ++v) {
        if (degV[v] <= 1) qv.push(v);
    }
    for (int e = 0; e < m; ++e) {
        push_edge(e);
    }

    auto subset_active = [&](int a, int b) -> bool {
        // Are the active vertices of bag a a subset of the active vertices of bag b?
        if (deadE[b]) return false;

        for (int v : verts[a]) {
            if (deadV[v]) continue;
            if (!std::binary_search(cover[v].begin(), cover[v].end(), b)) {
                return false;
            }
        }
        return true;
    };

    auto contained = [&](int e) -> bool {
        if (deadE[e]) return false;
        if (szE[e] == 0) return true;

        int piv = -1;
        for (int v : verts[e]) {
            if (deadV[v]) continue;
            if (piv == -1 || degV[v] < degV[piv]) {
                piv = v;
            }
        }

        if (piv == -1) return true;

        // Any active superset of e must contain piv, so only inspect bags covering piv.
        for (int f : cover[piv]) {
            if (f == e || deadE[f]) continue;
            if (szE[f] < szE[e]) continue;
            if (subset_active(e, f)) return true;
        }
        return false;
    };

    int removed = 0;

    while (!qv.empty() || !qe.empty()) {
        while (!qv.empty()) {
            int v = qv.front();
            qv.pop();

            if (deadV[v] || degV[v] > 1) continue;

            deadV[v] = 1;
            for (int e : cover[v]) {
                if (deadE[e]) continue;
                --szE[e];
                push_edge(e);
            }
        }

        while (!qe.empty()) {
            int e = qe.front();
            qe.pop();
            inQE[e] = 0;

            if (deadE[e]) continue;
            if (!contained(e)) continue;

            deadE[e] = 1;
            ++removed;

            for (int v : verts[e]) {
                if (deadV[v]) continue;
                --degV[v];
                push_vertex(v);
            }
        }
    }

    return removed == m;
}



/*
 * Given a set of bags that admit a join tree, construct one such join tree.
 *
 * Primary objective:
 *   maximize the sum of intersection sizes.
 *
 * Secondary objective:
 *   minimize the sum of AGM bounds of the graphs induced by the union
 *   of the two endpoint bags.
 *
 * total_join_cost:
 *   sum of the AGM costs of the selected join-tree edges.
 *
 * Output format:
 *   {root, list of children of each node}
 */
std::pair<int, std::vector<std::vector<int>>> recover_join_tree(
    const std::vector<std::vector<int>>& bags,
    int n,
    const std::vector<std::pair<int, int>>& query_edges,
    const std::vector<int>& weights,
    double& total_join_cost
) {
    const int B = static_cast<int>(bags.size());

    std::string mode = "zero";
    std::vector<std::vector<int>> children(B);

    total_join_cost = 0.0;

    if (query_edges.size() != weights.size()) {
        throw std::runtime_error(
            "recover_join_tree: query_edges and weights have different sizes"
        );
    }

    if (B == 0) {
        return {-1, children};
    }

    if (B == 1) {
        return {0, children};
    }

    struct DSU {
        std::vector<int> parent;
        std::vector<int> size;

        explicit DSU(int n)
            : parent(n), size(n, 1) {
            std::iota(parent.begin(), parent.end(), 0);
        }

        int find(int x) {
            if (parent[x] == x) {
                return x;
            }

            return parent[x] = find(parent[x]);
        }

        bool unite(int a, int b) {
            a = find(a);
            b = find(b);

            if (a == b) {
                return false;
            }

            if (size[a] < size[b]) {
                std::swap(a, b);
            }

            parent[b] = a;
            size[a] += size[b];

            return true;
        }
    };

    auto intersection_size = [&](int i, int j) {
        std::vector<char> seen(n, 0);

        for (int vertex : bags[i]) {
            seen[vertex] = 1;
        }

        int count = 0;

        for (int vertex : bags[j]) {
            if (seen[vertex]) {
                ++count;
            }
        }

        return count;
    };

    auto calculate_join_cost = [&](int i, int j) {
        std::vector<char> in_union(n, 0);

        for (int vertex : bags[i]) {
            in_union[vertex] = 1;
        }

        for (int vertex : bags[j]) {
            in_union[vertex] = 1;
        }

        std::vector<solver::Edge> induced_subgraph;
        induced_subgraph.reserve(query_edges.size());

        for (
            int edge_id = 0;
            edge_id < static_cast<int>(query_edges.size());
            ++edge_id
        ) {
            const int u = query_edges[edge_id].first;
            const int v = query_edges[edge_id].second;

            if (!in_union[u] || !in_union[v]) {
                continue;
            }

            solver::Edge induced_edge{};
            induced_edge.u = u;
            induced_edge.v = v;
            induced_edge.w = std::log2(
                static_cast<double>(
                    std::max(1, weights[edge_id])
                )
            );

            induced_subgraph.push_back(induced_edge);
        }

        if (induced_subgraph.empty()) {
            return 1.0;
        }

        solver::FractionalEdgeCoverSolver fec_solver;

        solver::Result result = fec_solver.solve(
            induced_subgraph,
            n
        );

        return std::exp2(result.objective_value);
    };

    struct Edge {
        int u;
        int v;
        int intersection;
        double join_cost;
    };

    std::vector<Edge> complete_graph_edges;
    complete_graph_edges.reserve(B * (B - 1) / 2);

    for (int i = 0; i < B; ++i) {
        for (int j = i + 1; j < B; ++j) {
            complete_graph_edges.push_back({
                i,
                j,
                intersection_size(i, j),
                calculate_join_cost(i, j)
            });
        }
    }

    /*
     * First maximize separator size, preserving the join-tree property.
     * For equal separator sizes, choose lower estimated join cost.
     */
    std::sort(
        complete_graph_edges.begin(),
        complete_graph_edges.end(),
        [](const Edge& a, const Edge& b) {
            if (a.intersection != b.intersection) {
                return a.intersection > b.intersection;
            }

            if (a.join_cost != b.join_cost) {
                return a.join_cost < b.join_cost;
            }

            if (a.u != b.u) {
                return a.u < b.u;
            }

            return a.v < b.v;
        }
    );

    DSU dsu(B);
    std::vector<std::vector<int>> undirected_tree(B);

    int selected_edges = 0;

    for (const Edge& edge : complete_graph_edges) {
        if (!dsu.unite(edge.u, edge.v)) {
            continue;
        }

        undirected_tree[edge.u].push_back(edge.v);
        undirected_tree[edge.v].push_back(edge.u);

        total_join_cost += edge.join_cost;

        ++selected_edges;

        if (selected_edges == B - 1) {
            break;
        }
    }

    if (selected_edges != B - 1) {
        throw std::runtime_error(
            "recover_join_tree produced a disconnected tree"
        );
    }

    if (mode == "centroid") {
        std::vector<int> parent(B, -1);
        std::vector<int> order;
        order.reserve(B);

        std::queue<int> queue;

        parent[0] = 0;
        queue.push(0);

        while (!queue.empty()) {
            const int u = queue.front();
            queue.pop();

            order.push_back(u);

            for (int v : undirected_tree[u]) {
                if (parent[v] != -1) {
                    continue;
                }

                parent[v] = u;
                queue.push(v);
            }
        }

        if (static_cast<int>(order.size()) != B) {
            throw std::runtime_error(
                "recover_join_tree produced a disconnected tree"
            );
        }

        std::vector<int> subtree_size(B, 1);

        for (int index = B - 1; index >= 0; --index) {
            const int u = order[index];

            for (int v : undirected_tree[u]) {
                if (parent[v] == u) {
                    subtree_size[u] += subtree_size[v];
                }
            }
        }

        int centroid = 0;
        int best_max_component = B + 1;

        for (int u = 0; u < B; ++u) {
            int max_component = B - subtree_size[u];

            for (int v : undirected_tree[u]) {
                if (parent[v] == u) {
                    max_component = std::max(
                        max_component,
                        subtree_size[v]
                    );
                }
            }

            if (max_component < best_max_component) {
                best_max_component = max_component;
                centroid = u;
            }
        }

        children.assign(B, std::vector<int>());
        std::fill(parent.begin(), parent.end(), -1);

        parent[centroid] = centroid;
        queue.push(centroid);

        while (!queue.empty()) {
            const int u = queue.front();
            queue.pop();

            for (int v : undirected_tree[u]) {
                if (parent[v] != -1) {
                    continue;
                }

                parent[v] = u;
                children[u].push_back(v);
                queue.push(v);
            }
        }

        return {centroid, children};
    }

    std::vector<int> parent(B, -1);
    std::queue<int> queue;

    parent[0] = 0;
    queue.push(0);

    while (!queue.empty()) {
        const int u = queue.front();
        queue.pop();

        for (int v : undirected_tree[u]) {
            if (parent[v] != -1) {
                continue;
            }

            parent[v] = u;
            children[u].push_back(v);
            queue.push(v);
        }
    }

    return {0, children};
}


} // namespace hypergraph

#include "project/hypergraphs/hypertree_check.hpp"

#include <algorithm>
#include <numeric>
#include <queue>
#include <vector>
#include <iostream>

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
 *  Given a set of bags that admit a join tree, construct one such join tree.
 *  output format: {root, list of children of each node}
 *
 */
std::pair<int, std::vector<std::vector<int>>> recover_join_tree(const std::vector<std::vector<int>>& bags, int n) {
int B = (int)bags.size();
    // can be centroid or zero, default is zero
    std::string mode = "zero";
    std::vector<std::vector<int>> children(B);

    if (B == 0) {
        return {-1, children};
    }

    if (B == 1) {
        return {0, children};
    }

    struct DSU {
        std::vector<int> p, sz;

        DSU(int n) : p(n), sz(n, 1) {
            std::iota(p.begin(), p.end(), 0);
        }

        int find(int x) {
            if (p[x] == x) return x;
            return p[x] = find(p[x]);
        }

        bool unite(int a, int b) {
            a = find(a);
            b = find(b);

            if (a == b) return false;

            if (sz[a] < sz[b]) std::swap(a, b);

            p[b] = a;
            sz[a] += sz[b];

            return true;
        }
    };

    auto intersection_size = [&](int i, int j) {
        std::vector<char> seen(n, 0);

        for (int x : bags[i]) {
            seen[x] = 1;
        }

        int cnt = 0;

        for (int x : bags[j]) {
            if (seen[x]) cnt++;
        }

        return cnt;
    };

    struct Edge {
        int u, v, w;
    };

    std::vector<Edge> complete_graph_edges;

    for (int i = 0; i < B; i++) {
        for (int j = i + 1; j < B; j++) {
            complete_graph_edges.push_back({
                i,
                j,
                intersection_size(i, j)
            });
        }
    }

    // Maximum spanning tree by intersection size.
    std::sort(
        complete_graph_edges.begin(),
        complete_graph_edges.end(),
        [](const Edge& a, const Edge& b) {
            return a.w > b.w;
        }
    );

    DSU dsu(B);
    std::vector<std::vector<int>> undirected_tree(B);

    for (const Edge& e : complete_graph_edges) {
        if (dsu.unite(e.u, e.v)) {
            undirected_tree[e.u].push_back(e.v);
            undirected_tree[e.v].push_back(e.u);
        }
    }

    // centroid version
    // Orient the tree from root 0.
    /*
     * Find a centroid of the undirected join tree.
     *
     * A centroid is a node c such that, after removing c,
     * every connected component has size at most B / 2.
     */
    
    if(mode == "centroid"){
        std::vector<int> parent(B, -1);
        std::vector<int> order;
        order.reserve(B);

        std::queue<int> q;
        parent[0] = 0;
        q.push(0);

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            order.push_back(u);

            for (int v : undirected_tree[u]) {
                if (parent[v] != -1) continue;

                parent[v] = u;
                q.push(v);
            }
        }

        if ((int)order.size() != B) {
            std::cout << "recorver_join_tree produced disconnected tree" << '\n';
        }

        std::vector<int> subtree_size(B, 1);

        for (int i = B - 1; i >= 0; i--) {
            int u = order[i];

            for (int v : undirected_tree[u]) {
                if (parent[v] == u) {
                    subtree_size[u] += subtree_size[v];
                }
            }
        }

        int centroid = 0;
        int best_max_component = B + 1;

        for (int u = 0; u < B; u++) {
            int max_component = B - subtree_size[u];

            for (int v : undirected_tree[u]) {
                if (parent[v] == u) {
                    max_component = std::max(max_component, subtree_size[v]);
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
        q.push(centroid);

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            for (int v : undirected_tree[u]) {
                if (parent[v] != -1) continue;

                parent[v] = u;
                children[u].push_back(v);
                q.push(v);
            }
        }

        return {centroid, children}; 
    }
    else{ // default rooting mode 
        std::vector<int> parent(B, -1);
        std::queue<int> q;

        parent[0] = 0;
        q.push(0);

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            for (int v : undirected_tree[u]) {
                if (parent[v] != -1) continue;

                parent[v] = u;
                children[u].push_back(v);
                q.push(v);
            }
        }

        return {0, children}; 
    }
}
} // namespace hypergraph
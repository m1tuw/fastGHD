//
// Created by anouk on 08-08-23.
//

#ifndef CCQ_QDAGS_GHD_HPP
#define CCQ_QDAGS_GHD_HPP

#include <inttypes.h>
#include <vector>

#include "ghd_solver.hpp"
#include "qdags.hpp"
#include "../src/joins.cpp"

using namespace std;

class ghd {
    vector<qdag> relations;
    vector<ghd> children;

public:
    ghd() = default;

    ghd(const std::vector<qdag>& qdags, const std::vector<ghd>& subtrees)
        : relations(qdags)
        , children(subtrees)
    {
    }

    const std::vector<qdag>& get_relations() const
    {
        return relations;
    }

    const std::vector<ghd>& get_children() const
    {
        return children;
    }

    vector<qdag> get_child_qdags() const
    {
        // This will be used during semijoin, so there will only be 1 qdag per vector
        // obtengo el primer qdag que guarda cada uno de mis hijos en su nodo
        vector<qdag> results;
        results.reserve(children.size());
        for (const auto& child : children) {
            results.push_back(child.get_relations().front());
        }
        return results;
    }

    void get_subtree_qdags(vector<qdag>& subtree) const
    {
        subtree.push_back(relations.front());
        for (const auto& child : children) {
            child.get_subtree_qdags(subtree);
        }
    }

    void collect_all_nodes(vector<ghd*>& subtree)
    {

        subtree.push_back(this);

        // Recursively collect nodes from children
        for (auto child = children.begin(); child != children.end(); child++) {
            child->collect_all_nodes(subtree);
        }
    }

    void set_relations(const vector<qdag> new_relations)
    {
        relations = new_relations;
    }

    void exec_multijoin()
    {
        // ejecuta multijoin entre las relaciones del nodo y reemplaza el vector de relaciones
        if (relations.size() == 1) {
            return;
        }
        qdag* join_result = multiJoin(relations, false, 1000);
        relations.clear();
        relations.push_back(*join_result);
        relations.shrink_to_fit();
    }

    void deep_exec_multijoin()
    {
        exec_multijoin();
        for (auto child = children.begin(); child != children.end(); child++) {
            child->deep_exec_multijoin();
        }
    }

    void constrained_by_children()
    {
        // si soy hoja empiezo a subir
        if (children.empty()) {
            return;
        } else {
            // bajo por el árbol
            for (auto child = children.begin(); child != children.end(); child++) {
                child->constrained_by_children();
            }
            // semijoin entre nodo y sus hijos. Debo pasarle un vector en el cual el primer elemento sea
            // mi qdag, y el resto son los qdag de children
            // esto debería alterar mi qdag
            vector<qdag> rels = get_child_qdags();
            rels.insert(rels.begin(), relations.front());
            semiJoin(rels, false, 1000);
        }
    }

    // constrain children
    // iterar sobre hijos y llamar semijoin entre hijo_i y nodo
    void constrain_children()
    {
        // si soy hoja termino
        if (children.empty()) {
            return;
        } else {
            vector<qdag> pair(2);
            pair[1] = relations.front();
            for (auto child = children.begin(); child != children.end(); child++) {
                // obtengo el qdag guardado en el hijo
                pair[0] = child->get_relations().front();
                // retrinjo al hijo
                semiJoin(pair, false, 1000);
                // bajo por el árbol
                child->constrain_children();
            }
        }
    }

    uint64_t size()
    {
        uint64_t total = 0;
        for (auto qdag = relations.begin(); qdag != relations.end(); qdag++) {
            total += qdag->size();
        }
        for (auto child = children.begin(); child != children.end(); child++) {
            total += child->size();
        }
        return total;
    }

    void print_n_ones(std::optional<std::reference_wrapper<std::ofstream>> outfile)
    {
        outfile->get() << n_ones();
        for (auto child = children.begin(); child != children.end(); child++) {
            outfile->get() << "-";
            child->print_n_ones(outfile);
        }
    }

    void print_all_ones(std::optional<std::reference_wrapper<std::ofstream>> outfile)
    {
        int n_tuples = 0;
        for (auto rel = relations.begin(); rel != relations.end(); rel++) {
            n_tuples += rel->n_ones();
        }
        outfile->get() << n_tuples;
        for (auto child = children.begin(); child != children.end(); child++) {
            outfile->get() << "-";
            child->print_all_ones(outfile);
        }
    }

    uint64_t n_ones()
    {
        return relations.front().n_ones();
    }

    ghd get_optimal_ghd(int number_of_nodes, int number_of_edges, const vector<pair<int,int>>& edges, vector<qdag> qdags, const vector<int>& weights) {
        GHDSolver solver;
        GHDSolver::GHDResult res = solver.solve(number_of_nodes, number_of_edges, edges, weights);
        const vector<vector<int>>& bags = res.bags;
        const vector<vector<int>>& join_tree = res.join_tree;

        int B = (int)bags.size();

        if (B == 0) {
            throw std::runtime_error("GHDSolver returned no bags");
        }

        if ((int)join_tree.size() != B) {
            throw std::runtime_error("join_tree size does not match number of bags");
        }

        /*
         * Precompute bag membership.
         * in_bag[b][v] = true iff vertex v is inside bag b.
         */
        vector<vector<char>> in_bag(B, vector<char>(number_of_nodes, 0));

        for (int b = 0; b < B; b++) {
            for (int v : bags[b]) {
                if (v < 0 || v >= number_of_nodes) {
                    throw std::runtime_error("bag contains vertex out of range");
                }
                in_bag[b][v] = 1;
            }
        }

        auto bag_contains_edge = [&](int b, int e) -> bool {
            int u = edges[e].first;
            int v = edges[e].second;

            if (u < 0 || u >= number_of_nodes || v < 0 || v >= number_of_nodes) {
                throw std::runtime_error("edge endpoint out of range");
            }

            return in_bag[b][u] && in_bag[b][v];
        };

        /*
         * relations_per_bag[b] stores the ids of qdags assigned to bag b.
         *
         * Important: this implementation of class ghd assumes each node has at least
         * one qdag, because several methods use relations.front().
         *
         * Therefore we first try to assign one unique edge to every bag.
         */
        vector<vector<int>> relations_per_bag(B);
        vector<int> assigned_edge(number_of_edges, -1);

        // First pass: try to give each bag at least one still-unassigned edge.
        for (int b = 0; b < B; b++) {
            for (int e = 0; e < number_of_edges; e++) {
                if (assigned_edge[e] == -1 && bag_contains_edge(b, e)) {
                    assigned_edge[e] = b;
                    relations_per_bag[b].push_back(e);
                    break;
                }
            }
        }

        // Second pass: assign all remaining edges to the first bag that contains them.
        for (int e = 0; e < number_of_edges; e++) {
            if (assigned_edge[e] != -1) {
                continue;
            }

            for (int b = 0; b < B; b++) {
                if (bag_contains_edge(b, e)) {
                    assigned_edge[e] = b;
                    relations_per_bag[b].push_back(e);
                    break;
                }
            }

            if (assigned_edge[e] == -1) {
                throw std::runtime_error("No bag contains both endpoints of an edge");
            }
        }

        /*
         * If a bag still has no relation, we duplicate any qdag whose edge is
         * contained in that bag.
         *
         * This is needed because the current ghd class cannot represent an
         * attribute-only bag. Duplicating an atom is logically harmless for the join,
         * but it can add some extra work.
         */
        for (int b = 0; b < B; b++) {
            if (!relations_per_bag[b].empty()) {
                continue;
            }

            for (int e = 0; e < number_of_edges; e++) {
                if (bag_contains_edge(b, e)) {
                    relations_per_bag[b].push_back(e);
                    break;
                }
            }

            if (relations_per_bag[b].empty()) {
                throw std::runtime_error("Bag has no contained edge/qdag to assign");
            }
        }

        /*
         * Recursively build the ghd tree.
         *
         * res.join_tree is assumed to be a rooted tree represented as:
         * join_tree[u] = list of children of u.
         */
        std::function<ghd(int)> build = [&](int node_id) -> ghd {
            vector<qdag> node_qdags;

            for (int edge_id : relations_per_bag[node_id]) {
                node_qdags.push_back(qdags[edge_id]);
            }

            vector<ghd> subtrees;

            for (int child_id : join_tree[node_id]) {
                if (child_id < 0 || child_id >= B) {
                    throw std::runtime_error("join_tree contains invalid child id");
                }

                subtrees.push_back(build(child_id));
            }

            return ghd(node_qdags, subtrees);
        };

        return build(0);
    }
};

#endif // CCQ_QDAGS_GHD_HPP
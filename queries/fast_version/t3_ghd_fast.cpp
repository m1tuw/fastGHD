#include <fstream>
#include <bits/stdc++.h>
#include <ratio>
#include <chrono>
#include <ctime>
#include <iostream>

using namespace std::chrono;
using namespace std;

#include "../includes/ghd.hpp"
#include "../src/ghd_optimal_joins.cpp"

#define AT_X 0
#define AT_Y 1
#define AT_Z 2
#define AT_V 3

int main(int argc, char** argv) {
    if (argc < 8) {
        cerr << "Usage: star_fast <R> <S> <T> <mode1> <mode2> <output> <dummy>" << endl;
        return 1;
    }

    qdag::att_set att_R;
    qdag::att_set att_S;
    qdag::att_set att_T;

    // Query:
    // R(0,1), S(0,2), T(0,3)
    att_R.push_back(AT_X);
    att_R.push_back(AT_Y);

    att_S.push_back(AT_X);
    att_S.push_back(AT_Z);

    att_T.push_back(AT_X);
    att_T.push_back(AT_V);

    /*
     * Grafo lógico de la query para el solver.
     *
     * edges[i] debe corresponder a qdags[i].
     *
     * qdags[0] = R(0,1)
     * qdags[1] = S(0,2)
     * qdags[2] = T(0,3)
     */
    int number_of_nodes = 4;
    int number_of_edges = 3;

    vector<pair<int, int>> edges;
    edges.emplace_back(AT_X, AT_Y); // R
    edges.emplace_back(AT_X, AT_Z); // S
    edges.emplace_back(AT_X, AT_V); // T

    string strRel_R(argv[1]);
    string strRel_S(argv[2]);
    string strRel_T(argv[3]);

    vector<vector<uint64_t>>* rel_R = read_relation(strRel_R, att_R.size());
    vector<vector<uint64_t>>* rel_S = read_relation(strRel_S, att_S.size());
    vector<vector<uint64_t>>* rel_T = read_relation(strRel_T, att_T.size());

    uint64_t grid_side = 32;

    grid_side = maximum_in_table(*rel_R, att_R.size(), grid_side);
    grid_side = maximum_in_table(*rel_S, att_S.size(), grid_side);
    grid_side = maximum_in_table(*rel_T, att_T.size(), grid_side);

    grid_side = pow(2, std::ceil(log2(grid_side)));

    qdag qdag_rel_R(*rel_R, att_R, grid_side, 2, att_R.size());
    qdag qdag_rel_S(*rel_S, att_S, grid_side, 2, att_S.size());
    qdag qdag_rel_T(*rel_T, att_T, grid_side, 2, att_T.size());

    auto rels = { rel_R, rel_S, rel_T };

    cout << "read all relations, with a total of "
         << relations_size(rels)
         << " tuples"
         << endl;

    vector<qdag> qdags(3);
    qdags[0] = qdag_rel_R;
    qdags[1] = qdag_rel_S;
    qdags[2] = qdag_rel_T;

    /*
     * Important:
     * Do NOT call sort_relations(qdags) here.
     *
     * The solver assumes:
     *
     *   edges[i] corresponds to qdags[i]
     *   weights[i] corresponds to qdags[i]
     */
    vector<int> weights;
    for (auto r : rels) {
        weights.push_back(r->size());
    }

    vector<string> edge_names = { "R", "S", "T" };

    /*
    cout << "edge weights:" << endl;
    for (int i = 0; i < number_of_edges; i++) {
        cout << "  qdags[" << i << "] = "
             << edge_names[i]
             << "(" << edges[i].first << "," << edges[i].second << ")"
             << " has weight "
             << weights[i]
             << endl;
    }
    cout << endl;
    */

    ghd root;
    root = root.get_optimal_ghd(
        number_of_nodes,
        number_of_edges,
        edges,
        qdags,
        weights
    );

    run_experiment(argv, argc, rels, qdags, root);

    return 0;
}
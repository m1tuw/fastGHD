//
// Created by anouk on 30-09-24.
//

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
#define AT_XP 3
#define AT_YP 4
#define AT_ZP 5

int main(int argc, char **argv) {
    if (argc < 11) {
        cerr << "Usage: square_tadpole_fast <P> <Q> <R> <S> <T> <U> <mode1> <mode2> <output> <dummy>" << endl;
        return 1;
    }

    qdag::att_set att_P;
    qdag::att_set att_Q;
    qdag::att_set att_R;
    qdag::att_set att_S;
    qdag::att_set att_T;
    qdag::att_set att_U;

    // Square:
    // P(0,1), Q(1,2), R(2,3), S(3,0)
    att_P.push_back(AT_X);
    att_P.push_back(AT_Y);

    att_Q.push_back(AT_Y);
    att_Q.push_back(AT_Z);

    att_R.push_back(AT_Z);
    att_R.push_back(AT_XP);

    att_S.push_back(AT_XP);
    att_S.push_back(AT_X);

    // Tail:
    // T(3,4), U(4,5)
    att_T.push_back(AT_XP);
    att_T.push_back(AT_YP);

    att_U.push_back(AT_YP);
    att_U.push_back(AT_ZP);

    /*
     * Grafo lógico de la query para el solver.
     *
     * edges[i] debe corresponder a qdags[i].
     *
     * qdags[0] = P(0,1)
     * qdags[1] = Q(1,2)
     * qdags[2] = R(2,3)
     * qdags[3] = S(3,0)
     * qdags[4] = T(3,4)
     * qdags[5] = U(4,5)
     */
    int number_of_nodes = 6;
    int number_of_edges = 6;

    vector<pair<int, int>> edges;
    edges.emplace_back(AT_X,  AT_Y);  // P
    edges.emplace_back(AT_Y,  AT_Z);  // Q
    edges.emplace_back(AT_Z,  AT_XP); // R
    edges.emplace_back(AT_XP, AT_X);  // S
    edges.emplace_back(AT_XP, AT_YP); // T
    edges.emplace_back(AT_YP, AT_ZP); // U

    string strRel_P(argv[1]);
    string strRel_Q(argv[2]);
    string strRel_R(argv[3]);
    string strRel_S(argv[4]);
    string strRel_T(argv[5]);
    string strRel_U(argv[6]);

    vector<vector<uint64_t>>* rel_P = read_relation(strRel_P, att_P.size());
    vector<vector<uint64_t>>* rel_Q = read_relation(strRel_Q, att_Q.size());
    vector<vector<uint64_t>>* rel_R = read_relation(strRel_R, att_R.size());
    vector<vector<uint64_t>>* rel_S = read_relation(strRel_S, att_S.size());
    vector<vector<uint64_t>>* rel_T = read_relation(strRel_T, att_T.size());
    vector<vector<uint64_t>>* rel_U = read_relation(strRel_U, att_U.size());

    uint64_t grid_side = 128;

    grid_side = maximum_in_table(*rel_P, att_P.size(), grid_side);
    grid_side = maximum_in_table(*rel_Q, att_Q.size(), grid_side);
    grid_side = maximum_in_table(*rel_R, att_R.size(), grid_side);
    grid_side = maximum_in_table(*rel_S, att_S.size(), grid_side);
    grid_side = maximum_in_table(*rel_T, att_T.size(), grid_side);
    grid_side = maximum_in_table(*rel_U, att_U.size(), grid_side);

    grid_side = pow(2, std::ceil(log2(grid_side)));

    qdag qdag_rel_P(*rel_P, att_P, grid_side, 2, att_P.size());
    qdag qdag_rel_Q(*rel_Q, att_Q, grid_side, 2, att_Q.size());
    qdag qdag_rel_R(*rel_R, att_R, grid_side, 2, att_R.size());
    qdag qdag_rel_S(*rel_S, att_S, grid_side, 2, att_S.size());
    qdag qdag_rel_T(*rel_T, att_T, grid_side, 2, att_T.size());
    qdag qdag_rel_U(*rel_U, att_U, grid_side, 2, att_U.size());

    auto rels = { rel_P, rel_Q, rel_R, rel_S, rel_T, rel_U };

    cout << "read all relations, with a total of "
         << relations_size(rels)
         << " tuples"
         << endl;

    vector<qdag> qdags(6);
    qdags[0] = qdag_rel_P;
    qdags[1] = qdag_rel_Q;
    qdags[2] = qdag_rel_R;
    qdags[3] = qdag_rel_S;
    qdags[4] = qdag_rel_T;
    qdags[5] = qdag_rel_U;

    vector<int> weights;
    for(auto r: rels){
        weights.push_back(r->size());
    }

    ghd root;
    root = root.get_optimal_ghd(number_of_nodes, number_of_edges, edges, qdags, weights);

    run_experiment(argv, argc, rels, qdags, root);

    return 0;
}
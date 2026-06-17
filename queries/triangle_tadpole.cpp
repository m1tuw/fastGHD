//
// Created by anouk on 15-01-25.
//

#include <fstream>
#include<bits/stdc++.h>
#include<ratio>
#include<chrono>
#include<ctime>
#include<iostream>

using namespace std::chrono;

#include "../includes/ghd.hpp"
#include "../src/ghd_optimal_joins.cpp"



#define AT_X 0
#define AT_Y 1
#define AT_Z 2
#define AT_XP 3
#define AT_YP 4
#define AT_ZP 5


int main(int argc, char **argv) {
    // Setup de GHD: leer qdags que forman nodos
    qdag::att_set att_R;
    qdag::att_set att_S;
    qdag::att_set att_T;
    qdag::att_set att_P;
    qdag::att_set att_Q;
    qdag::att_set att_U;

    att_P.push_back(0); att_P.push_back(1);
    att_Q.push_back(1); att_Q.push_back(2);
    att_R.push_back(2); att_R.push_back(0);
    att_T.push_back(2); att_T.push_back(3);
    att_U.push_back(3); att_U.push_back(4);

    std::string strRel_P(argv[1]), strRel_Q(argv[2]), strRel_R(argv[3]), strRel_T(argv[4]), strRel_U(argv[5]);

    std::vector<std::vector<uint64_t>>* rel_P = read_relation(strRel_P, att_P.size());
    std::vector<std::vector<uint64_t>>* rel_Q = read_relation(strRel_Q, att_Q.size());
    std::vector<std::vector<uint64_t>>* rel_R = read_relation(strRel_R, att_R.size());
    //cout<<"R"<<endl;
    std::vector<std::vector<uint64_t>>* rel_T = read_relation(strRel_T, att_T.size());
    std::vector<std::vector<uint64_t>>* rel_U = read_relation(strRel_U, att_U.size());
    //cout<<"t"<<endl;
    //std::vector<std::vector<uint64_t>>* rel_U = read_relation(strRel_U, att_U.size());

    uint64_t grid_side = 128;

    grid_side = maximum_in_table(*rel_P, att_P.size(), grid_side);
    grid_side = maximum_in_table(*rel_Q, att_Q.size(), grid_side);
    grid_side = maximum_in_table(*rel_R, att_R.size(), grid_side);
    grid_side = maximum_in_table(*rel_T, att_T.size(), grid_side);
    grid_side = maximum_in_table(*rel_U, att_U.size(), grid_side);

    grid_side = pow(2, std::ceil(log2(grid_side) ));
    //cout << grid_side << endl;
    qdag qdag_rel_P(*rel_P, att_P, grid_side, 2, att_P.size());
    //cout << "Built P\n";
    qdag qdag_rel_Q(*rel_Q, att_Q, grid_side, 2, att_Q.size());
    //cout << "Built Q\n";
    qdag qdag_rel_R(*rel_R, att_R, grid_side, 2, att_R.size());
    //cout << "Built R\n";
    qdag qdag_rel_T(*rel_T, att_T, grid_side, 2, att_T.size());
    qdag qdag_rel_U(*rel_U, att_U, grid_side, 2, att_U.size());
    //cout << "Built T\n";
auto rels = { rel_P, rel_Q, rel_R, rel_T, rel_U };
    std::cout << "read all relations, with a total of " << relations_size(rels) << " tuples" << endl;

    vector<qdag> qdags(5);
    qdags[0] = qdag_rel_P;
    qdags[1] = qdag_rel_Q;
    qdags[2] = qdag_rel_R;
    qdags[3] = qdag_rel_T;
    qdags[4] = qdag_rel_U;
    
    ghd root;
    vector<qdag> Q_root(3);

    Q_root[0] = qdag_rel_P;
    Q_root[1] = qdag_rel_Q;
    Q_root[2] = qdag_rel_R;

    vector<qdag> Q_b(2);
    Q_b[0] = qdag_rel_T;
    Q_b[1] = qdag_rel_U;

    // Crear GHDs

    vector<ghd> empty_children(0);
    ghd sub_b = ghd(Q_b, empty_children);
    //ghd sub_c = ghd(Q_c, empty_children);
    vector<ghd> level_1;
    level_1.push_back(sub_b);

    //level_1.push_back(sub_c);
    root = ghd(Q_root, level_1);
    
    run_experiment(argv, argc, rels, qdags, root);
}
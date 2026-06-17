//
// Created by anouk on 31-12-24.
//


#include <fstream>
#include<bits/stdc++.h>
#include<ratio>
#include<chrono>
#include<ctime>

using namespace std::chrono;


#include "../includes/ghd.hpp"
#include "../src/ghd_optimal_joins.cpp"


#define AT_X 0
#define AT_Y 1
#define AT_Z 2
#define AT_V 3
#define AT_U 4

int main(int argc, char** argv)
{
    qdag::att_set att_R;
    qdag::att_set att_S;
    qdag::att_set att_T;
    qdag::att_set att_U;

    att_R.push_back(AT_Y); att_R.push_back(AT_X);
    att_S.push_back(AT_Z); att_S.push_back(AT_X);
    att_T.push_back(AT_X); att_T.push_back(AT_U);
    att_U.push_back(AT_X); att_U.push_back(AT_V);

    std::string strRel_R(argv[1]), strRel_S(argv[2]), strRel_T(argv[3]), strRel_U(argv[4]);

    std::vector<std::vector<uint64_t>>* rel_R = read_relation(strRel_R, att_R.size());
    std::vector<std::vector<uint64_t>>* rel_S = read_relation(strRel_S, att_S.size());
    std::vector<std::vector<uint64_t>>* rel_T = read_relation(strRel_T, att_T.size());
    std::vector<std::vector<uint64_t>>* rel_U = read_relation(strRel_U, att_U.size());

    uint64_t grid_side = 0;

    grid_side = maximum_in_table(*rel_R, att_R.size(), grid_side);
    grid_side = maximum_in_table(*rel_S, att_S.size(), grid_side);
    grid_side = maximum_in_table(*rel_T, att_T.size(), grid_side);
    grid_side = maximum_in_table(*rel_U, att_U.size(), grid_side);

    grid_side = pow(2, std::ceil(log2(grid_side) ));

    //cout << "Grid side: " << grid_side << endl;

    qdag qdag_rel_R(*rel_R, att_R, grid_side, 2, att_R.size());
    qdag qdag_rel_S(*rel_S, att_S, grid_side, 2, att_S.size());
    qdag qdag_rel_T(*rel_T, att_T, grid_side, 2, att_T.size());
    qdag qdag_rel_U(*rel_U, att_U, grid_side, 2, att_U.size());

    auto rels = { rel_R, rel_S, rel_T, rel_U};
    std::cout << "read all relations, with a total of " << relations_size(rels) << " tuples" << endl;

    vector<qdag> qdags = { qdag_rel_R, qdag_rel_S, qdag_rel_T, qdag_rel_U};
    ghd root;
    if (strcmp(argv[argc - 1], "1") == 0) {
        vector<qdag> Q_root(2);
        Q_root[0] = qdag_rel_R;
        Q_root[1] = qdag_rel_S;

        vector<qdag> Q_b(2);
        Q_b[0] = qdag_rel_T;
        Q_b[1] = qdag_rel_U;

        // Crear GHDs
        vector<ghd> empty_children(0);
        ghd sub_b = ghd(Q_b, empty_children);
        vector<ghd> level_1;
        level_1.push_back(sub_b);
        root = ghd(Q_root, level_1);

    }
    else if (strcmp(argv[argc - 1], "2") == 0){
        vector<qdag> Q_root(2);
        Q_root[0] = qdag_rel_R;
        Q_root[1] = qdag_rel_T;

        vector<qdag> Q_b(2);
        Q_b[0] = qdag_rel_S;
        Q_b[1] = qdag_rel_U;

        // Crear GHDs
        vector<ghd> empty_children(0);
        ghd sub_b = ghd(Q_b, empty_children);
        vector<ghd> level_1;
        level_1.push_back(sub_b);
        root = ghd(Q_root, level_1);
    }
    else if (strcmp(argv[argc - 1], "3") == 0){
        vector<qdag> Q_root(2);
        Q_root[0] = qdag_rel_R;
        Q_root[1] = qdag_rel_U;

        vector<qdag> Q_b(2);
        Q_b[0] = qdag_rel_S;
        Q_b[1] = qdag_rel_T;

        // Crear GHDs
        vector<ghd> empty_children(0);
        ghd sub_b = ghd(Q_b, empty_children);
        vector<ghd> level_1;
        level_1.push_back(sub_b);
        root = ghd(Q_root, level_1);
    }
    else if (strcmp(argv[argc - 1], "4") == 0){
        vector<qdag> Q_root(1);
        Q_root[0] = qdag_rel_R;

        vector<qdag> Q_b(3);
        Q_b[0] = qdag_rel_S;
        Q_b[1] = qdag_rel_T;
        Q_b[2] = qdag_rel_U;

        // Crear GHDs
        vector<ghd> empty_children(0);
        ghd sub_b = ghd(Q_b, empty_children);
        vector<ghd> level_1;
        level_1.push_back(sub_b);
        root = ghd(Q_root, level_1);
    }
    else if (strcmp(argv[argc - 1], "5") == 0){
        vector<qdag> Q_root(1);
        Q_root[0] = qdag_rel_S;

        vector<qdag> Q_b(3);
        Q_b[0] = qdag_rel_R;
        Q_b[1] = qdag_rel_T;
        Q_b[2] = qdag_rel_U;

        // Crear GHDs
        vector<ghd> empty_children(0);
        ghd sub_b = ghd(Q_b, empty_children);
        vector<ghd> level_1;
        level_1.push_back(sub_b);
        root = ghd(Q_root, level_1);
    }
    else if (strcmp(argv[argc - 1], "6") == 0){
        vector<qdag> Q_root(1);
        Q_root[0] = qdag_rel_T;

        vector<qdag> Q_b(3);
        Q_b[0] = qdag_rel_S;
        Q_b[1] = qdag_rel_R;
        Q_b[2] = qdag_rel_U;

        // Crear GHDs
        vector<ghd> empty_children(0);
        ghd sub_b = ghd(Q_b, empty_children);
        vector<ghd> level_1;
        level_1.push_back(sub_b);
        root = ghd(Q_root, level_1);
    }
    else if (strcmp(argv[argc - 1], "7") == 0){
        vector<qdag> Q_root(1);
        Q_root[0] = qdag_rel_U;

        vector<qdag> Q_b(3);
        Q_b[0] = qdag_rel_S;
        Q_b[1] = qdag_rel_T;
        Q_b[2] = qdag_rel_R;

        // Crear GHDs
        vector<ghd> empty_children(0);
        ghd sub_b = ghd(Q_b, empty_children);
        vector<ghd> level_1;
        level_1.push_back(sub_b);
        root = ghd(Q_root, level_1);
    }
    run_experiment(argv, argc, rels, qdags, root);


    return 0;
}
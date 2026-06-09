
#include <fstream>
#include<bits/stdc++.h>
#include<ratio>
#include<chrono>
#include<ctime>

using namespace std::chrono;


//#include "../src/joins.cpp"
#include "../includes/ghd.hpp"
#include "../src/ghd_optimal_joins.cpp"


#define AT_A 0
#define AT_B 1
#define AT_C 2
#define AT_D 3
#define AT_E 4
#define AT_F 5
#define AT_G 6
#define AT_H 7


int main(int argc, char **argv) {
    // Setup de GHD: leer qdags que forman nodos
    qdag::att_set att_R;
    qdag::att_set att_S;
    qdag::att_set att_T;
    qdag::att_set att_U;
    qdag::att_set att_J;
    qdag::att_set att_A;
    qdag::att_set att_B;
    qdag::att_set att_C;
    qdag::att_set att_D;

    att_A.push_back(0);
    att_A.push_back(1);

    att_B.push_back(1);
    att_B.push_back(2);

    att_C.push_back(2);
    att_C.push_back(3);

    att_D.push_back(3);
    att_D.push_back(0);

    att_J.push_back(3);
    att_J.push_back(4);

    att_R.push_back(4);
    att_R.push_back(5);

    att_S.push_back(5);
    att_S.push_back(6);

    att_T.push_back(6);
    att_T.push_back(7);

    att_U.push_back(7);
    att_U.push_back(4);

    std::string strRel_A(argv[1]), strRel_B(argv[2]), strRel_C(argv[3]), strRel_D(argv[4]), strRel_J(argv[5]), strRel_R(argv[6]), strRel_S(argv[7]), strRel_T(argv[8]), strRel_U(argv[9]);

    std::vector<std::vector<uint64_t>> *rel_R = read_relation(strRel_R, att_R.size());
    std::vector<std::vector<uint64_t>> *rel_S = read_relation(strRel_S, att_S.size());
    std::vector<std::vector<uint64_t>> *rel_T = read_relation(strRel_T, att_T.size());
    std::vector<std::vector<uint64_t>> *rel_U = read_relation(strRel_U, att_U.size());
    std::vector<std::vector<uint64_t>> *rel_A = read_relation(strRel_A, att_A.size());
    std::vector<std::vector<uint64_t>> *rel_B = read_relation(strRel_B, att_B.size());
    std::vector<std::vector<uint64_t>> *rel_C = read_relation(strRel_C, att_C.size());
    std::vector<std::vector<uint64_t>> *rel_D = read_relation(strRel_D, att_D.size());
    std::vector<std::vector<uint64_t>> *rel_J = read_relation(strRel_J, att_J.size());

    uint64_t grid_side = 32;

    grid_side = maximum_in_table(*rel_A, att_A.size(), grid_side);
    grid_side = maximum_in_table(*rel_B, att_B.size(), grid_side);
    grid_side = maximum_in_table(*rel_C, att_C.size(), grid_side);
    grid_side = maximum_in_table(*rel_D, att_D.size(), grid_side);
    grid_side = maximum_in_table(*rel_J, att_J.size(), grid_side);
    grid_side = maximum_in_table(*rel_R, att_R.size(), grid_side);
    grid_side = maximum_in_table(*rel_S, att_S.size(), grid_side);
    grid_side = maximum_in_table(*rel_T, att_T.size(), grid_side);
    grid_side = maximum_in_table(*rel_U, att_U.size(), grid_side);

    grid_side = pow(2, std::ceil(log2(grid_side) ));

    qdag qdag_rel_A(*rel_A, att_A, grid_side, 2, att_A.size());
    qdag qdag_rel_B(*rel_B, att_B, grid_side, 2, att_B.size());
    qdag qdag_rel_C(*rel_C, att_C, grid_side, 2, att_C.size());
    qdag qdag_rel_D(*rel_D, att_D, grid_side, 2, att_D.size());

    qdag qdag_rel_J(*rel_J, att_J, grid_side, 2, att_J.size());

    qdag qdag_rel_R(*rel_R, att_R, grid_side, 2, att_R.size());
    qdag qdag_rel_S(*rel_S, att_S, grid_side, 2, att_S.size());
    qdag qdag_rel_T(*rel_T, att_T, grid_side, 2, att_T.size());
    qdag qdag_rel_U(*rel_U, att_U, grid_side, 2, att_U.size());
    // Crear vectores de relacion de cada nodo

    auto rels = { rel_A, rel_B, rel_C, rel_D, rel_J, rel_R, rel_S, rel_T, rel_U };
    std::cout << "read all relations, with a total of " << relations_size(rels) << " tuples" << endl;
    vector<qdag> qdags(9);
    qdags[0] = qdag_rel_A;
    qdags[1] = qdag_rel_B;
    qdags[2] = qdag_rel_C;
    qdags[3] = qdag_rel_D;
    qdags[4] = qdag_rel_J;
    qdags[5] = qdag_rel_R;
    qdags[6] = qdag_rel_S;
    qdags[7] = qdag_rel_T;
    qdags[8] = qdag_rel_U;
    ghd root;
    vector<qdag> Q_root(1);
    Q_root[0] = qdag_rel_J;

    vector<qdag> Q_b(4);
    Q_b[0] = qdag_rel_A;
    Q_b[1] = qdag_rel_B;
    Q_b[2] = qdag_rel_C;
    Q_b[3] = qdag_rel_D;

    vector<qdag> Q_c(4);
    Q_c[0] = qdag_rel_R;
    Q_c[1] = qdag_rel_S;
    Q_c[2] = qdag_rel_T;
    Q_c[3] = qdag_rel_U;

    // Crear GHDs
    vector<ghd> empty_children(0);
    ghd sub_b = ghd(Q_b, empty_children);
    ghd sub_c = ghd(Q_c, empty_children);
    vector<ghd> level_1;
    level_1.push_back(sub_b);
    level_1.push_back(sub_c);
    root = ghd(Q_root, level_1);

    run_experiment(argv, argc, rels, qdags, root);
    return 0;
}

#include <fstream>
#include<bits/stdc++.h>
#include<ratio>

using namespace std::chrono;

#include "../includes/ghd.hpp"
#include "../src/ghd_optimal_joins.cpp"


#define AT_X 0
#define AT_Y 1
#define AT_Z 2
#define AT_V 3


int main(int argc, char** argv)
{
    qdag::att_set att_R;
    qdag::att_set att_S;
    qdag::att_set att_T;

    att_R.push_back(1); att_R.push_back(0); 
    att_S.push_back(2); att_S.push_back(0); 
    att_T.push_back(3); att_T.push_back(0);

    std::string strRel_R(argv[1]), strRel_S(argv[2]), strRel_T(argv[3]);

    std::vector<std::vector<uint64_t>>* rel_R = read_relation(strRel_R, att_R.size());
    std::vector<std::vector<uint64_t>>* rel_S = read_relation(strRel_S, att_S.size());
    std::vector<std::vector<uint64_t>>* rel_T = read_relation(strRel_T, att_T.size());

    // uint64_t grid_side = 52000000; // es como +infty para wikidata
    uint64_t grid_side = 0;
    grid_side = maximum_in_table(*rel_R, att_R.size(), grid_side);
    grid_side = maximum_in_table(*rel_S, att_S.size(), grid_side);
    grid_side = maximum_in_table(*rel_T, att_T.size(), grid_side);
    grid_side = pow(2, std::ceil(log2(grid_side)));

    qdag qdag_rel_R(*rel_R, att_R, grid_side, 2, att_R.size());
    qdag qdag_rel_S(*rel_S, att_S, grid_side, 2, att_S.size());
    qdag qdag_rel_T(*rel_T, att_T, grid_side, 2, att_T.size());

    auto rels = { rel_R, rel_S, rel_T };
    std::cout << "read all relations, with a total of " << relations_size(rels) << " tuples" << endl;

    vector<qdag> qdags = { qdag_rel_R, qdag_rel_S, qdag_rel_T };
    sort_relations(qdags);
    ghd root;
        if (strcmp(argv[argc - 1], "1") == 0) {
            // root tiene el minimo
            vector<qdag> Q_root(1);
            Q_root[0] = qdags[0];

            vector<qdag> Q_b(2);
            Q_b[0] = qdags[1];
            Q_b[1] = qdags[2];

            // Crear GHDs
            vector<ghd> empty_children(0);
            ghd sub_b = ghd(Q_b, empty_children);
            vector<ghd> level_1;
            level_1.push_back(sub_b);
            root = ghd(Q_root, level_1);

        }
        else if (strcmp(argv[argc - 1], "2") == 0){
            // root tiene el mediano
            vector<qdag> Q_root(1);
            Q_root[0] = qdags[1];

            vector<qdag> Q_b(2);
            Q_b[0] = qdags[0];
            Q_b[1] = qdags[2];

            // Crear GHDs
            vector<ghd> empty_children(0);
            ghd sub_b = ghd(Q_b, empty_children);
            vector<ghd> level_1;
            level_1.push_back(sub_b);
            root = ghd(Q_root, level_1);
        }
        else if (strcmp(argv[argc - 1], "3") == 0){
            // root tiene el maximo
            vector<qdag> Q_root(1);
            Q_root[0] = qdags[2];

            vector<qdag> Q_b(2);
            Q_b[0] = qdags[0];
            Q_b[1] = qdags[1];
            // Crear GHDs
            vector<ghd> empty_children(0);
            ghd sub_b = ghd(Q_b, empty_children);
            vector<ghd> level_1;
            level_1.push_back(sub_b);
            root = ghd(Q_root, level_1);
        }
        else if (strcmp(argv[argc - 1], "4") == 0){
        // root tiene el maximo
        vector<qdag> Q_root(2);
        Q_root[0] = qdags[0];
        Q_root[1] = qdags[1];

        vector<qdag> Q_b(2);
        Q_b[0] = qdags[0];
        Q_b[1] = qdags[2];

        // Crear GHDs
        vector<ghd> empty_children(0);
        ghd sub_b = ghd(Q_b, empty_children);
        vector<ghd> level_1;
        level_1.push_back(sub_b);
        root = ghd(Q_root, level_1);
    }
    else if (strcmp(argv[argc - 1], "5") == 0){
        // root tiene el maximo
        vector<qdag> Q_root(2);
        Q_root[0] = qdags[1];
        Q_root[1] = qdags[0];

        vector<qdag> Q_b(2);
        Q_b[0] = qdags[1];
        Q_b[1] = qdags[2];

        // Crear GHDs
        vector<ghd> empty_children(0);
        ghd sub_b = ghd(Q_b, empty_children);
        vector<ghd> level_1;
        level_1.push_back(sub_b);
        root = ghd(Q_root, level_1);
    }
    else if (strcmp(argv[argc - 1], "6") == 0){
        // root tiene el maximo
        vector<qdag> Q_root(2);
        Q_root[0] = qdags[2];
        Q_root[1] = qdags[0];

        vector<qdag> Q_b(2);
        Q_b[0] = qdags[2];
        Q_b[1] = qdags[1];

        // Crear GHDs
        vector<ghd> empty_children(0);
        ghd sub_b = ghd(Q_b, empty_children);
        vector<ghd> level_1;
        level_1.push_back(sub_b);
        root = ghd(Q_root, level_1);
    }
    else if (strcmp(argv[argc - 1], "7") == 0){
        // root tiene el maximo
        vector<qdag> Q_root(1);
        Q_root[0] = qdags[0];

        vector<qdag> Q_b(1);
        Q_b[0] = qdags[1];
        vector<qdag> Q_c(1);
        Q_c[0] = qdags[2];

        // Crear GHDs
        vector<ghd> empty_children(0);
        ghd sub_b = ghd(Q_b, empty_children);
        ghd sub_c = ghd(Q_c, empty_children);
        vector<ghd> level_1;
        level_1.push_back(sub_b);
        level_1.push_back(sub_c);
        root = ghd(Q_root, level_1);
    }
    else if (strcmp(argv[argc - 1], "8") == 0){
        // root tiene el maximo
        vector<qdag> Q_root(1);
        Q_root[0] = qdags[1];

        vector<qdag> Q_b(1);
        Q_b[0] = qdags[0];
        vector<qdag> Q_c(1);
        Q_c[0] = qdags[2];

        // Crear GHDs
        vector<ghd> empty_children(0);
        ghd sub_b = ghd(Q_b, empty_children);
        ghd sub_c = ghd(Q_c, empty_children);
        vector<ghd> level_1;
        level_1.push_back(sub_b);
        level_1.push_back(sub_c);
        root = ghd(Q_root, level_1);
    }
    else if (strcmp(argv[argc - 1], "9") == 0){
        // root tiene el maximo
        vector<qdag> Q_root(1);
        Q_root[0] = qdags[2];

        vector<qdag> Q_b(1);
        Q_b[0] = qdags[1];
        vector<qdag> Q_c(1);
        Q_c[0] = qdags[0];

        // Crear GHDs
        vector<ghd> empty_children(0);
        ghd sub_b = ghd(Q_b, empty_children);
        ghd sub_c = ghd(Q_c, empty_children);
        vector<ghd> level_1;
        level_1.push_back(sub_b);
        level_1.push_back(sub_c);
        root = ghd(Q_root, level_1);
    }

    run_experiment(argv, argc, rels, qdags, root);
    return 0;
}
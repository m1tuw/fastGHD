#include <fstream>
#include <bits/stdc++.h>
#include <ratio>

using namespace std::chrono;
using namespace std;

#include "../includes/ghd.hpp"
#include "../src/ghd_optimal_joins.cpp"

#define AT_X 0
#define AT_Y 1
#define AT_Z 2
#define AT_YP 3
#define AT_ZP 4

int main(int argc, char** argv)
{
    // example usage: bowtie_fast R S T RP SP TP time yk results.csv 0
    if (argc < 10) {
        cerr << "Usage: bowtie_fast <R> <S> <T> <RP> <SP> <TP> <mode1> <mode2> <output> <dummy>" << endl;
        return 1;
    }

    // Setup de GHD: leer qdags que forman nodos
    qdag::att_set att_R;
    qdag::att_set att_S;
    qdag::att_set att_T;
    qdag::att_set att_RP;
    qdag::att_set att_SP;
    qdag::att_set att_TP;

    att_R.push_back(AT_X);
    att_R.push_back(AT_Y);

    att_S.push_back(AT_Y);
    att_S.push_back(AT_Z);

    att_T.push_back(AT_Z);
    att_T.push_back(AT_X);

    att_RP.push_back(AT_X);
    att_RP.push_back(AT_YP);

    att_SP.push_back(AT_YP);
    att_SP.push_back(AT_ZP);

    att_TP.push_back(AT_ZP);
    att_TP.push_back(AT_X);

    // representar el grafo como lista de aristas para pasarselo al solver
    int number_of_nodes = 5;
    int number_of_edges = 6;
    vector<pair<int,int>> edges;
    edges.emplace_back(AT_X, AT_Y);
    edges.emplace_back(AT_Y, AT_Z);
    edges.emplace_back(AT_Z, AT_X);
    edges.emplace_back(AT_X, AT_YP);
    edges.emplace_back(AT_YP, AT_ZP);
    edges.emplace_back(AT_ZP, AT_X);

    std::string strRel_R(argv[1]), strRel_S(argv[2]), strRel_T(argv[3]), strRel_RP(argv[4]), strRel_SP(argv[5]), strRel_TP(argv[6]);

    std::vector<std::vector<uint64_t>>* rel_R = read_relation(strRel_R, att_R.size());
    std::vector<std::vector<uint64_t>>* rel_S = read_relation(strRel_S, att_S.size());
    std::vector<std::vector<uint64_t>>* rel_T = read_relation(strRel_T, att_T.size());
    std::vector<std::vector<uint64_t>>* rel_RP = read_relation(strRel_RP, att_RP.size());
    std::vector<std::vector<uint64_t>>* rel_SP = read_relation(strRel_SP, att_SP.size());
    std::vector<std::vector<uint64_t>>* rel_TP = read_relation(strRel_TP, att_TP.size());

    uint64_t grid_side = 32;

    grid_side = maximum_in_table(*rel_R, att_R.size(), grid_side);
    grid_side = maximum_in_table(*rel_S, att_S.size(), grid_side);
    grid_side = maximum_in_table(*rel_T, att_T.size(), grid_side);
    grid_side = maximum_in_table(*rel_RP, att_RP.size(), grid_side);
    grid_side = maximum_in_table(*rel_SP, att_SP.size(), grid_side);
    grid_side = maximum_in_table(*rel_TP, att_TP.size(), grid_side);

    grid_side = pow(2, std::ceil(log2(grid_side)));

    qdag qdag_rel_R(*rel_R, att_R, grid_side, 2, att_R.size());
    qdag qdag_rel_S(*rel_S, att_S, grid_side, 2, att_S.size());
    qdag qdag_rel_T(*rel_T, att_T, grid_side, 2, att_T.size());
    qdag qdag_rel_RP(*rel_RP, att_RP, grid_side, 2, att_RP.size());
    qdag qdag_rel_SP(*rel_SP, att_SP, grid_side, 2, att_SP.size());
    qdag qdag_rel_TP(*rel_TP, att_TP, grid_side, 2, att_TP.size());

    auto rels = { rel_R, rel_S, rel_T, rel_RP, rel_SP, rel_TP };
    cout << "read all relations, with a total of " << relations_size(rels) << " tuples" << endl;

    // one qdag for each edge in this case
    vector<qdag> qdags(6);
    qdags[0] = qdag_rel_R;
    qdags[1] = qdag_rel_S;
    qdags[2] = qdag_rel_T;
    qdags[3] = qdag_rel_RP;
    qdags[4] = qdag_rel_SP;
    qdags[5] = qdag_rel_TP;

    ghd root;
    vector<qdag> Q_root(3);
    Q_root[0] = qdag_rel_R;
    Q_root[1] = qdag_rel_S;
    Q_root[2] = qdag_rel_T;

    vector<qdag> Q_c(3);
    Q_c[0] = qdag_rel_RP;
    Q_c[1] = qdag_rel_SP;
    Q_c[2] = qdag_rel_TP;

    // Crear GHDs
    root = ghd();
    root = root.get_optimal_ghd(number_of_nodes, number_of_edges, edges, qdags);

    run_experiment(argv, argc, rels, qdags, root);
    return 0;
}
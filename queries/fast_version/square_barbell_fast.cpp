#include <fstream>
#include <bits/stdc++.h>
#include <ratio>
#include <chrono>
#include <ctime>

using namespace std::chrono;
using namespace std;

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
    if (argc < 14) {
        cerr << "Usage: square_barbell_fast <A> <B> <C> <D> <J> <R> <S> <T> <U> <mode1> <mode2> <output> <dummy>" << endl;
        return 1;
    }

    qdag::att_set att_A;
    qdag::att_set att_B;
    qdag::att_set att_C;
    qdag::att_set att_D;
    qdag::att_set att_J;
    qdag::att_set att_R;
    qdag::att_set att_S;
    qdag::att_set att_T;
    qdag::att_set att_U;

    // Primer 4-ciclo:
    // A(0,1), B(1,2), C(2,3), D(3,0)
    att_A.push_back(AT_A);
    att_A.push_back(AT_B);

    att_B.push_back(AT_B);
    att_B.push_back(AT_C);

    att_C.push_back(AT_C);
    att_C.push_back(AT_D);

    att_D.push_back(AT_D);
    att_D.push_back(AT_A);

    // Arista puente:
    // J(3,4)
    att_J.push_back(AT_D);
    att_J.push_back(AT_E);

    // Segundo 4-ciclo:
    // R(4,5), S(5,6), T(6,7), U(7,4)
    att_R.push_back(AT_E);
    att_R.push_back(AT_F);

    att_S.push_back(AT_F);
    att_S.push_back(AT_G);

    att_T.push_back(AT_G);
    att_T.push_back(AT_H);

    att_U.push_back(AT_H);
    att_U.push_back(AT_E);

    /*
     * Grafo lógico de la query para pasárselo al solver.
     *
     * Importante:
     * edges[i] debe corresponder a qdags[i].
     */
    int number_of_nodes = 8;
    int number_of_edges = 9;

    vector<pair<int, int>> edges;
    edges.emplace_back(AT_A, AT_B); // qdags[0] = A
    edges.emplace_back(AT_B, AT_C); // qdags[1] = B
    edges.emplace_back(AT_C, AT_D); // qdags[2] = C
    edges.emplace_back(AT_D, AT_A); // qdags[3] = D
    edges.emplace_back(AT_D, AT_E); // qdags[4] = J
    edges.emplace_back(AT_E, AT_F); // qdags[5] = R
    edges.emplace_back(AT_F, AT_G); // qdags[6] = S
    edges.emplace_back(AT_G, AT_H); // qdags[7] = T
    edges.emplace_back(AT_H, AT_E); // qdags[8] = U

    std::string strRel_A(argv[1]);
    std::string strRel_B(argv[2]);
    std::string strRel_C(argv[3]);
    std::string strRel_D(argv[4]);
    std::string strRel_J(argv[5]);
    std::string strRel_R(argv[6]);
    std::string strRel_S(argv[7]);
    std::string strRel_T(argv[8]);
    std::string strRel_U(argv[9]);

    std::vector<std::vector<uint64_t>> *rel_A = read_relation(strRel_A, att_A.size());
    std::vector<std::vector<uint64_t>> *rel_B = read_relation(strRel_B, att_B.size());
    std::vector<std::vector<uint64_t>> *rel_C = read_relation(strRel_C, att_C.size());
    std::vector<std::vector<uint64_t>> *rel_D = read_relation(strRel_D, att_D.size());
    std::vector<std::vector<uint64_t>> *rel_J = read_relation(strRel_J, att_J.size());
    std::vector<std::vector<uint64_t>> *rel_R = read_relation(strRel_R, att_R.size());
    std::vector<std::vector<uint64_t>> *rel_S = read_relation(strRel_S, att_S.size());
    std::vector<std::vector<uint64_t>> *rel_T = read_relation(strRel_T, att_T.size());
    std::vector<std::vector<uint64_t>> *rel_U = read_relation(strRel_U, att_U.size());

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

    grid_side = pow(2, std::ceil(log2(grid_side)));

    qdag qdag_rel_A(*rel_A, att_A, grid_side, 2, att_A.size());
    qdag qdag_rel_B(*rel_B, att_B, grid_side, 2, att_B.size());
    qdag qdag_rel_C(*rel_C, att_C, grid_side, 2, att_C.size());
    qdag qdag_rel_D(*rel_D, att_D, grid_side, 2, att_D.size());
    qdag qdag_rel_J(*rel_J, att_J, grid_side, 2, att_J.size());
    qdag qdag_rel_R(*rel_R, att_R, grid_side, 2, att_R.size());
    qdag qdag_rel_S(*rel_S, att_S, grid_side, 2, att_S.size());
    qdag qdag_rel_T(*rel_T, att_T, grid_side, 2, att_T.size());
    qdag qdag_rel_U(*rel_U, att_U, grid_side, 2, att_U.size());

    auto rels = { rel_A, rel_B, rel_C, rel_D, rel_J, rel_R, rel_S, rel_T, rel_U };

    std::cout << "read all relations, with a total of "
              << relations_size(rels)
              << " tuples"
              << endl;

    /*
     * Un qdag por átomo/arista de la query.
     *
     * El orden debe coincidir con edges:
     *
     * qdags[0] = A(0,1)
     * qdags[1] = B(1,2)
     * qdags[2] = C(2,3)
     * qdags[3] = D(3,0)
     * qdags[4] = J(3,4)
     * qdags[5] = R(4,5)
     * qdags[6] = S(5,6)
     * qdags[7] = T(6,7)
     * qdags[8] = U(7,4)
     */
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

    vector<int> weights;
    

    /*
     * Ya no hardcodeamos:
     *
     *        {J}
     *       /   \
     *   {A,B,C,D} {R,S,T,U}
     *
     * Ahora el solver calcula los bags y el join tree.
     */
    ghd root;
    root = root.get_optimal_ghd(number_of_nodes, number_of_edges, edges, qdags);

    run_experiment(argv, argc, rels, qdags, root);

    return 0;
}
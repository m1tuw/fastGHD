//
// Created by anouk on 24-04-21.
//
#include "ghd.hpp"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <omp.h>
#include <ratio>
#include <optional>

using namespace std::chrono;

high_resolution_clock::time_point start_select, stop_select;
double total_time_select = 0.0;
duration<double> time_span_select;

qdag* yannakakis(ghd root, std::optional<std::reference_wrapper<std::ofstream>> outfile)
{
    float init, mid, end;
    
    init = root.size();
    //cout << "GHD size before multijoins:" << root.size() << endl;
    // Ejecutar multijoin en todos los niveles
    root.deep_exec_multijoin();

    //cout << "GHD size after multijoins:" << root.size() << endl;
    mid = root.size();
    // Ejecutar semijoin entre root y nivel 1
    root.constrained_by_children();

    root.constrain_children();

    // multijoin entre nodos para obtener resultado del join
    vector<qdag> producto_punto;
    root.get_subtree_qdags(producto_punto);

    qdag* qResult = multiJoin(producto_punto, false, 1000);
    end = qResult->size();
    if(outfile){
        outfile->get() << init << "," << mid << "," << end;
    }
    
    return qResult;
}

qdag* yannakakis_par(ghd root)
{
    // Collect all nodes into a flat list
    std::vector<ghd*> node_list;
    root.collect_all_nodes(node_list);

    // Execute exec_multijoin in parallel for all nodes
    int j = 2;
    #pragma omp parallel for num_threads(j)
        for (size_t i = 0; i < node_list.size(); ++i) {
            node_list[i]->exec_multijoin();
        }
    

    // solve_mj = high_resolution_clock::now();

    // Ejecutar semijoin entre root y nivel 1

    root.constrained_by_children();
    // bottom_up = high_resolution_clock::now();

    root.constrain_children();

    // multijoin entre nodos para obtener resultado del join
    // top_down = high_resolution_clock::now();
    vector<qdag> producto_punto;
    root.get_subtree_qdags(producto_punto);

    qdag* qResult = multiJoin(producto_punto, false, 1000);
    return qResult;
}

// Utilities to run queries
using namespace std::chrono;

vector<vector<uint64_t>>* read_relation(const string filename, uint16_t n_Atts)
{
    ifstream input_stream(filename);
    uint64_t x;
    uint16_t i, j = 0;

    vector<vector<uint64_t>>* relation;
    vector<uint64_t> tuple;

    relation = new vector<vector<uint64_t>>();
    if (!input_stream.good()) {
        cout << "file does not exist: '" << filename << "'" << endl;
        return relation;
    }

    input_stream >> x;
    while (!input_stream.eof()) {
        tuple.clear();
        for (i = 0; i < n_Atts; i++) {
            tuple.push_back(x);
            input_stream >> x;
        }
        relation->push_back(tuple);
    }

    return relation;
}

uint64_t maximum_in_table(vector<vector<uint64_t>>& table, uint16_t n_columns, uint64_t max_temp)
{
    uint64_t i, j;
    for (i = 0; i < table.size(); i++)
        for (j = 0; j < n_columns; j++)
            if (table[i][j] > max_temp)
                max_temp = table[i][j];

    return max_temp;
}


uint64_t relations_size(vector<vector<vector<uint64_t>>*> rels)
{
    uint64_t size = 0;
    for (auto rel : rels) {
        size += rel->size();
    }
    return size;
}

void sort_relations(vector<qdag>& qdags)
{
    int n = qdags.size();
    bool swapped;
    for (int i = 0; i < n - 1; i++) {
        swapped = false;
        for (int j = 0; j < n - i - 1; j++) {
            if (qdags[j].n_ones() > qdags[j + 1].n_ones()) {
                swap(qdags[j], qdags[j + 1]);
                swapped = true;
            }
        }
        if (!swapped)
            break;
    }
}

uint64_t qdags_size(vector<qdag> qdags)
{
    uint64_t size = 0;
    for (auto q : qdags) {
        size += q.size();
    }
    return size;
}

void run_experiment(char** argv, int argc, vector<vector<vector<uint64_t>>*> rels, vector<qdag> qdags, ghd root)
{
    ofstream outfile(argv[argc - 2], ios::app);

    if (strcmp(argv[argc - 4], "space") == 0) {
        outfile << relations_size(rels) << ",";
        outfile << qdags_size(qdags) << ",";
    }

    high_resolution_clock::time_point start, stop;

    if (strcmp(argv[argc - 3], "mj") == 0) {
        start = high_resolution_clock::now();
        qdag* test_result = multiJoin(qdags, false, 1000);
        stop = high_resolution_clock::now();
    } else {
        qdag* yan_res;

        if (strcmp(argv[argc - 3], "yk") == 0) {
            if (strcmp(argv[argc - 4], "space") == 0) {
                start = high_resolution_clock::now();
                yan_res = yannakakis(root, { outfile });
                stop = high_resolution_clock::now();
            } else {
                start = high_resolution_clock::now();
                yan_res = yannakakis(root, {});
                stop = high_resolution_clock::now();
            }
        } else {
            start = high_resolution_clock::now();
            yan_res = yannakakis_par(root);
            stop = high_resolution_clock::now();
        }
        if (strcmp(argv[argc - 4], "space") == 0) {
        outfile << yan_res->Q->bv[yan_res->getHeight()-1].n_ones();
    }
    }

    const std::chrono::duration<double, std::milli> time_span = stop - start;
    double time = time_span.count() / 1000;
    if (strcmp(argv[argc - 4], "time") == 0) {
        outfile << time;
        cout << "took " << time << "s" << endl;
    }
    outfile << endl;

    outfile.close();
}
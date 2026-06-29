#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;
using namespace std::chrono;

#include "../includes/ghd.hpp"
#include "../src/ghd_optimal_joins.cpp"

/*
 * Generic RDF-style query processor.
 *
 * Supported query atom format:
 *
 *      ?x1 1652 ?x2 .
 *      ?x2 2043 ?x3 .
 *
 * Each atom is treated as a binary relation:
 *
 *      P1652(?x1, ?x2)
 *      P2043(?x2, ?x3)
 *
 * The predicate id is mapped to a file using:
 *
 *      <data_dir>/prop-direct-P<predicate>
 *
 * Example:
 *
 *      1652 -> data/prop-direct-P1652
 *
 * The processor can:
 *
 *      - parse an arbitrary number of atoms/triples;
 *      - build one qdag per query atom;
 *      - compute a GHD using your GHDSolver;
 *      - run yannakakis or direct multiJoin;
 *      - dump materialized result tuples sorted by variable name;
 *      - print debug cardinality;
 *      - append benchmark rows to a CSV file.
 */


/*
 * One parsed RDF-style atom.
 *
 * subject_name and object_name are variables such as "?x1".
 * predicate is a string because filenames use the original predicate token.
 *
 * subject_id and object_id are filled after variable-id assignment.
 */
struct QueryAtom {
    string subject_name;
    string predicate;
    string object_name;

    int subject_id = -1;
    int object_id = -1;
};


/*
 * Runtime options parsed from argv.
 */
struct Options {
    string query_file;
    string data_dir = "data";
    string output_file = "query_results.tsv";
    string benchmark_file;
    string mode = "yk";

    bool debug = false;
    bool dump_results = true;
};


/*
 * Full parsed query.
 *
 * var_names[id] gives the variable name for that attribute id.
 * Atoms use subject_id/object_id into var_names.
 */
struct ParsedQuery {
    vector<QueryAtom> atoms;
    vector<string> var_names;
};


/*
 * The objects needed to execute a query.
 *
 * owned_relations keeps relation memory alive.
 * qdags has one qdag per atom, in exactly the same order as query.atoms.
 * edges is the logical query graph passed to the GHD solver.
 */
struct BuiltQuery {
    vector<unique_ptr<vector<vector<uint64_t>>>> owned_relations;
    vector<qdag> qdags;
    vector<pair<int, int>> edges;
    uint64_t grid_side = 1;
};


/*
 * Print usage and exit cleanly.
 */
static void print_usage(const char* program_name) {
    cerr
        << "Usage:\n"
        << "  " << program_name
        << " --query <query_file>"
        << " [--data-dir data]"
        << " [--output results.tsv]"
        << " [--mode yk|mj]"
        << " [--debug]"
        << " [--no-dump]"
        << " [--bench benchmarks.csv]\n\n"
        << "Example:\n"
        << "  " << program_name
        << " --query queries/path.query"
        << " --data-dir data"
        << " --output results/path.tsv"
        << " --mode yk"
        << " --debug"
        << " --bench results/bench.csv\n";
}


/*
 * Minimal command-line parser.
 *
 * I kept this manual instead of adding a dependency, because your project
 * already has enough build dependencies from SDSL and the solver.
 */
static Options parse_options(int argc, char** argv) {
    Options opt;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];

        auto require_value = [&](const string& flag) -> string {
            if (i + 1 >= argc) {
                throw runtime_error("Missing value after " + flag);
            }
            return argv[++i];
        };

        if (arg == "--query") {
            opt.query_file = require_value(arg);
        } else if (arg == "--data-dir") {
            opt.data_dir = require_value(arg);
        } else if (arg == "--output") {
            opt.output_file = require_value(arg);
        } else if (arg == "--mode") {
            opt.mode = require_value(arg);
        } else if (arg == "--bench") {
            opt.benchmark_file = require_value(arg);
        } else if (arg == "--debug") {
            opt.debug = true;
        } else if (arg == "--no-dump") {
            opt.dump_results = false;
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            exit(0);
        } else {
            throw runtime_error("Unknown argument: " + arg);
        }
    }

    if (opt.query_file.empty()) {
        throw runtime_error("Missing required argument: --query");
    }

    if (opt.mode != "yk" && opt.mode != "mj") {
        throw runtime_error("--mode must be either 'yk' or 'mj'");
    }

    return opt;
}


/*
 * Remove a trailing dot from a token.
 *
 * This allows both:
 *
 *      ?x1 1652 ?x2 .
 *
 * and:
 *
 *      ?x1 1652 ?x2.
 */
static string strip_trailing_dot(string token) {
    if (!token.empty() && token.back() == '.') {
        token.pop_back();
    }
    return token;
}


/*
 * Tokenize a query file.
 *
 * We remove standalone "." tokens because triples are parsed as groups of 3:
 *
 *      subject predicate object
 *
 * This is robust to the query being written on one line or many lines.
 */
static vector<string> read_query_tokens(const string& filename) {
    ifstream in(filename);

    if (!in.good()) {
        throw runtime_error("Could not open query file: " + filename);
    }

    vector<string> tokens;
    string token;

    while (in >> token) {
        if (token == ".") {
            continue;
        }

        token = strip_trailing_dot(token);

        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    return tokens;
}


/*
 * Parse RDF-style triples.
 *
 * The current qdag engine expects relations over variables. Therefore this
 * first version requires subject and object to be variables beginning with '?'.
 *
 * Constants can be added later by filtering relation files before building qdags.
 */
static ParsedQuery parse_query_file(const string& filename) {
    vector<string> tokens = read_query_tokens(filename);

    if (tokens.empty()) {
        throw runtime_error("Query file is empty");
    }

    if (tokens.size() % 3 != 0) {
        throw runtime_error(
            "Malformed query: expected token count to be a multiple of 3"
        );
    }

    ParsedQuery query;
    vector<string> variables;

    for (size_t i = 0; i < tokens.size(); i += 3) {
        QueryAtom atom;
        atom.subject_name = tokens[i];
        atom.predicate = tokens[i + 1];
        atom.object_name = tokens[i + 2];

        if (atom.subject_name.empty() || atom.subject_name[0] != '?') {
            throw runtime_error(
                "Only variable subjects are supported for now: " + atom.subject_name
            );
        }

        if (atom.object_name.empty() || atom.object_name[0] != '?') {
            throw runtime_error(
                "Only variable objects are supported for now: " + atom.object_name
            );
        }

        variables.push_back(atom.subject_name);
        variables.push_back(atom.object_name);
        query.atoms.push_back(atom);
    }

    /*
     * Alphabetical variable order.
     *
     * This makes output columns deterministic and satisfies the requirement
     * that results are dumped sorted alphabetically by variable name.
     *
     * Note: lexicographic sorting means ?x10 comes before ?x2.
     * If you want natural numeric ordering, change this comparator.
     */
    sort(variables.begin(), variables.end());
    variables.erase(unique(variables.begin(), variables.end()), variables.end());

    query.var_names = variables;

    unordered_map<string, int> id_of_variable;

    for (int i = 0; i < (int)query.var_names.size(); ++i) {
        id_of_variable[query.var_names[i]] = i;
    }

    for (QueryAtom& atom : query.atoms) {
        atom.subject_id = id_of_variable.at(atom.subject_name);
        atom.object_id = id_of_variable.at(atom.object_name);
    }

    return query;
}


/*
 * Build the data file path for one predicate.
 *
 * Input predicate "1652" becomes:
 *
 *      data/prop-direct-P1652
 *
 * If the user writes "P1652", we avoid generating "PP1652".
 */
static string predicate_to_file(const string& data_dir, const string& predicate) {
    string property_name;

    if (!predicate.empty() && predicate[0] == 'P') {
        property_name = predicate;
    } else {
        property_name = "P" + predicate;
    }

    return data_dir + "/prop-direct-" + property_name;
}


/*
 * Compute a power of two that is strictly greater than the maximum coordinate.
 *
 * The quadtree expects coordinates in [0, grid_side). If the maximum coordinate
 * is exactly a power of two, using that same value as grid_side would be wrong.
 */
static uint64_t next_power_of_two_strict(uint64_t max_value) {
    uint64_t side = 1;

    while (side <= max_value) {
        side <<= 1;
    }

    return side;
}


/*
 * Load every distinct predicate relation once.
 *
 * Even if the same predicate appears in many atoms, the file is read once.
 */
static unordered_map<string, vector<vector<uint64_t>>*> load_relations(
    const ParsedQuery& query,
    const Options& opt,
    vector<unique_ptr<vector<vector<uint64_t>>>>& owned_relations,
    uint64_t& max_value
) {
    unordered_map<string, vector<vector<uint64_t>>*> relation_by_predicate;

    max_value = 0;

    for (const QueryAtom& atom : query.atoms) {
        if (relation_by_predicate.count(atom.predicate)) {
            continue;
        }

        string path = predicate_to_file(opt.data_dir, atom.predicate);

        unique_ptr<vector<vector<uint64_t>>> relation(
            read_relation(path, 2)
        );

        if (relation->empty()) {
            throw runtime_error(
                "Relation file is empty or missing: " + path
            );
        }

        max_value = maximum_in_table(*relation, 2, max_value);

        relation_by_predicate[atom.predicate] = relation.get();
        owned_relations.push_back(std::move(relation));
    }

    return relation_by_predicate;
}


/*
 * Build one qdag per query atom.
 *
 * This is the generic replacement for hardcoded code such as:
 *
 *      att_R = [AT_X, AT_Y]
 *      qdag_rel_R(*rel_R, att_R, ...)
 *
 * Here, every atom generates:
 *
 *      att = [subject_id, object_id]
 *      qdag(relation_for_predicate, att, ...)
 */
static BuiltQuery build_query(const ParsedQuery& query, const Options& opt) {
    BuiltQuery built;

    uint64_t max_value = 0;

    auto relation_by_predicate = load_relations(
        query,
        opt,
        built.owned_relations,
        max_value
    );

    built.grid_side = next_power_of_two_strict(max_value);

    for (const QueryAtom& atom : query.atoms) {
        qdag::att_set attributes;

        attributes.push_back((uint64_t)atom.subject_id);
        attributes.push_back((uint64_t)atom.object_id);

        vector<vector<uint64_t>>& relation =
            *relation_by_predicate.at(atom.predicate);

        built.qdags.emplace_back(
            relation,
            attributes,
            built.grid_side,
            2,
            attributes.size()
        );

        built.edges.emplace_back(atom.subject_id, atom.object_id);
    }

    return built;
}


/*
 * Decode a child index into one digit per dimension.
 *
 * This mirrors se_quadtree::get_chunk_idx:
 *
 *      child = digit_0 * k^(d-1) + digit_1 * k^(d-2) + ... + digit_{d-1}
 *
 * For k = 2 and d = 3, child 5 = binary 101 -> [1, 0, 1].
 */
static vector<uint64_t> decode_child_index(uint64_t child, uint8_t k, uint8_t d) {
    vector<uint64_t> digits(d, 0);

    uint64_t divisor = 1;

    for (int i = 1; i < d; ++i) {
        divisor *= k;
    }

    for (int i = 0; i < d; ++i) {
        digits[i] = child / divisor;
        child %= divisor;

        if (divisor > 1) {
            divisor /= k;
        }
    }

    return digits;
}


/*
 * Recursively enumerate points from a qdag's underlying quadtree.
 *
 * This materializes the compressed result. Use it for correctness checks or
 * small/medium outputs, not for huge benchmark-only runs.
 *
 * The traversal uses public qdag::Q and se_quadtree::get_node/get_node_lastlevel.
 * Those are already the same primitives used by the join code.
 */
static void enumerate_qdag_dfs(
    qdag& result,
    uint16_t level,
    uint64_t node_start,
    uint64_t cell_size,
    vector<uint64_t>& offset,
    vector<vector<uint64_t>>& tuples
) {
    se_quadtree* tree = result.Q;

    const uint8_t k = tree->getK();
    const uint8_t d = result.nAttr();
    const uint64_t kd = tree->getKD();
    const uint16_t height = tree->getHeight();

    uint64_t mask = 0;
    vector<uint64_t> rank_array(kd, 0);

    if (level + 1 == height) {
        mask = tree->get_node_lastlevel(level, node_start);
    } else {
        uint64_t rank_value = tree->rank(level, node_start);
        mask = tree->get_node(level, node_start, rank_array.data(), rank_value);
    }

    for (uint64_t child = 0; child < kd; ++child) {
        if ((mask & (1ULL << child)) == 0) {
            continue;
        }

        vector<uint64_t> digits = decode_child_index(child, k, d);

        vector<uint64_t> next_offset = offset;

        for (uint8_t i = 0; i < d; ++i) {
            next_offset[i] += digits[i] * cell_size;
        }

        if (level + 1 == height) {
            tuples.push_back(next_offset);
        } else {
            uint64_t child_node_start = kd * (rank_array[child] - 1);
            enumerate_qdag_dfs(
                result,
                level + 1,
                child_node_start,
                cell_size / k,
                next_offset,
                tuples
            );
        }
    }
}


/*
 * Materialize all tuples from a qdag.
 *
 * The tuple order is the qdag attribute order. We later reorder columns by
 * alphabetical variable name before dumping.
 */
static vector<vector<uint64_t>> materialize_qdag(qdag& result) {
    vector<vector<uint64_t>> tuples;

    if (result.n_ones() == 0) {
        return tuples;
    }

    uint8_t k = result.getK();
    uint16_t height = result.getHeight();

    uint64_t initial_cell_size = 1;

    for (int i = 1; i < height; ++i) {
        initial_cell_size *= k;
    }

    vector<uint64_t> offset(result.nAttr(), 0);

    enumerate_qdag_dfs(
        result,
        0,
        0,
        initial_cell_size,
        offset,
        tuples
    );

    return tuples;
}


/*
 * Reorder one result tuple into alphabetical variable order.
 *
 * In this processor, variable ids are assigned alphabetically, so this usually
 * matches the result qdag order already. This function still checks explicitly,
 * making the dump robust if the qdag attribute order changes later.
 */
static vector<uint64_t> reorder_tuple_by_variable_id(
    qdag& result,
    const vector<uint64_t>& tuple,
    int variable_count
) {
    vector<uint64_t> reordered(variable_count, 0);

    for (uint64_t i = 0; i < result.nAttr(); ++i) {
        int variable_id = (int)result.getAttr(i);
        reordered[variable_id] = tuple[i];
    }

    return reordered;
}


/*
 * Dump all materialized tuples to a TSV file.
 *
 * Header columns are variable names sorted alphabetically.
 * Data rows are sorted lexicographically by tuple values.
 */
static void dump_results_sorted(
    qdag& result,
    const ParsedQuery& query,
    const string& output_file
) {
    vector<vector<uint64_t>> raw_tuples = materialize_qdag(result);
    vector<vector<uint64_t>> rows;

    rows.reserve(raw_tuples.size());

    for (const vector<uint64_t>& tuple : raw_tuples) {
        rows.push_back(
            reorder_tuple_by_variable_id(
                result,
                tuple,
                (int)query.var_names.size()
            )
        );
    }

    sort(rows.begin(), rows.end());

    ofstream out(output_file);

    if (!out.good()) {
        throw runtime_error("Could not open output file: " + output_file);
    }

    for (int i = 0; i < (int)query.var_names.size(); ++i) {
        if (i > 0) {
            out << '\t';
        }
        out << query.var_names[i];
    }
    out << '\n';

    for (const vector<uint64_t>& row : rows) {
        for (int i = 0; i < (int)row.size(); ++i) {
            if (i > 0) {
                out << '\t';
            }
            out << row[i];
        }
        out << '\n';
    }
}


/*
 * Append one benchmark row.
 *
 * The benchmark file is deliberately simple CSV:
 *
 *      query_file,mode,atoms,variables,time_seconds,cardinality
 */
static void append_benchmark(
    const Options& opt,
    const ParsedQuery& query,
    double seconds,
    uint64_t cardinality
) {
    if (opt.benchmark_file.empty()) {
        return;
    }

    bool file_already_exists = false;

    {
        ifstream test(opt.benchmark_file);
        file_already_exists = test.good() && test.peek() != ifstream::traits_type::eof();
    }

    ofstream out(opt.benchmark_file, ios::app);

    if (!out.good()) {
        throw runtime_error("Could not open benchmark file: " + opt.benchmark_file);
    }

    if (!file_already_exists) {
        out << "query_file,mode,atoms,variables,time_seconds,cardinality\n";
    }

    out << opt.query_file << ','
        << opt.mode << ','
        << query.atoms.size() << ','
        << query.var_names.size() << ','
        << fixed << setprecision(9) << seconds << ','
        << cardinality << '\n';
}


/*
 * Execute the query using the selected mode.
 *
 * mode = "mj":
 *      direct multiJoin over all atom qdags.
 *
 * mode = "yk":
 *      compute an optimal GHD using your solver, then run yannakakis.
 */
static qdag* execute_query(
    const ParsedQuery& query,
    BuiltQuery& built,
    const Options& opt
) {
    if (opt.mode == "mj") {
        return multiJoin(built.qdags, false, 1000);
    }

    ghd root;
    root = root.get_optimal_ghd(
        (int)query.var_names.size(),
        (int)query.atoms.size(),
        built.edges,
        built.qdags
    );

    return yannakakis(root, {});
}


int main(int argc, char** argv) {
    try {
        Options opt = parse_options(argc, argv);

        ParsedQuery query = parse_query_file(opt.query_file);

        if (opt.debug) {
            cerr << "Parsed query:\n";
            cerr << "  atoms:     " << query.atoms.size() << '\n';
            cerr << "  variables: " << query.var_names.size() << '\n';

            for (int i = 0; i < (int)query.var_names.size(); ++i) {
                cerr << "    var[" << i << "] = " << query.var_names[i] << '\n';
            }
        }

        BuiltQuery built = build_query(query, opt);

        if (opt.debug) {
            cerr << "Built query:\n";
            cerr << "  qdags:     " << built.qdags.size() << '\n';
            cerr << "  grid_side: " << built.grid_side << '\n';
        }

        high_resolution_clock::time_point start = high_resolution_clock::now();

        qdag* result = execute_query(query, built, opt);

        high_resolution_clock::time_point stop = high_resolution_clock::now();

        const duration<double> elapsed = stop - start;
        double seconds = elapsed.count();

        uint64_t cardinality = result->n_ones();

        if (opt.debug) {
            cerr << "Execution finished:\n";
            cerr << "  mode:        " << opt.mode << '\n';
            cerr << "  time:        " << fixed << setprecision(9) << seconds << "s\n";
            cerr << "  cardinality: " << cardinality << '\n';
        }

        append_benchmark(opt, query, seconds, cardinality);

        if (opt.dump_results) {
            dump_results_sorted(*result, query, opt.output_file);

            if (opt.debug) {
                cerr << "Results dumped to: " << opt.output_file << '\n';
            }
        }

        return 0;
    } catch (const exception& e) {
        cerr << "error: " << e.what() << '\n';
        print_usage(argv[0]);
        return 1;
    }
}
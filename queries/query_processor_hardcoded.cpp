#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <queue>
#include <stdexcept>
#include <string>
#include <sys/resource.h>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;
using namespace std::chrono;

#include "../includes/ghd.hpp"
#include "../src/ghd_optimal_joins.cpp"

struct QueryAtom {
    string subject_name;
    string predicate;
    string object_name;
    int subject_id = -1;
    int object_id = -1;
};

struct ParsedQuery {
    vector<QueryAtom> atoms;
    vector<string> var_names;
};

struct Options {
    string query_file;
    string data_dir = "data";
    string benchmark_file = "results/hardcoded_decompositions.csv";
    string shape;
    int decomposition_id = -1;
    int query_id = -1;
    int repetition = 1;
    bool debug = false;
};

struct LoadedQuery {
    ParsedQuery query;
    vector<unique_ptr<vector<vector<uint64_t>>>> owned_relations;
    unordered_map<string, vector<vector<uint64_t>>*> relation_by_predicate;
    uint64_t grid_side = 1;
};

struct HardcodedPlan {
    int id = -1;
    string name;
    vector<int> root_relations;
    vector<vector<int>> child_relations;
};

struct Timing {
    double wall_seconds = 0.0;
    double user_seconds = 0.0;
    double system_seconds = 0.0;
};

static void print_usage(const char* program_name) {
    cerr
        << "Usage:\n"
        << "  " << program_name
        << " --query <single_query_file>"
        << " --shape <j3|j4|t3|t4|ti3|ti4>"
        << " --decomposition <id>"
        << " [--data-dir data]"
        << " [--bench results/hardcoded_decompositions.csv]"
        << " [--query-id N]"
        << " [--repetition N]"
        << " [--debug]\n\n"
        << "The measured interval contains only yannakakis(root, {}).\n";
}

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
        } else if (arg == "--bench") {
            opt.benchmark_file = require_value(arg);
        } else if (arg == "--shape") {
            opt.shape = require_value(arg);
        } else if (arg == "--decomposition") {
            opt.decomposition_id = stoi(require_value(arg));
        } else if (arg == "--query-id") {
            opt.query_id = stoi(require_value(arg));
        } else if (arg == "--repetition") {
            opt.repetition = stoi(require_value(arg));
        } else if (arg == "--debug") {
            opt.debug = true;
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
    if (opt.shape.empty()) {
        throw runtime_error("Missing required argument: --shape");
    }
    if (opt.decomposition_id < 1) {
        throw runtime_error("Missing or invalid --decomposition id");
    }
    if (opt.repetition < 1) {
        throw runtime_error("--repetition must be positive");
    }

    return opt;
}

static double timeval_to_seconds(const timeval& value) {
    return static_cast<double>(value.tv_sec)
         + static_cast<double>(value.tv_usec) / 1000000.0;
}

static string strip_trailing_dot(string token) {
    while (!token.empty() && token.back() == '.') {
        token.pop_back();
    }
    return token;
}

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

static vector<string> preferred_variable_order(const string& shape) {
    if (shape == "j3") {
        return {"?x", "?y", "?z", "?v"};
    }
    if (shape == "t3" || shape == "ti3") {
        return {"?x", "?y", "?z", "?u"};
    }
    if (shape == "j4") {
        // Matches queries/j4_ghd.cpp: X=0, Y=1, Z=2, V=3, U=4.
        return {"?x", "?y", "?z", "?v", "?u"};
    }
    if (shape == "t4" || shape == "ti4") {
        return {"?x", "?y", "?z", "?u", "?v"};
    }
    throw runtime_error("Unsupported shape: " + shape);
}

static int expected_atom_count(const string& shape) {
    if (shape == "j3" || shape == "t3" || shape == "ti3") {
        return 3;
    }
    if (shape == "j4" || shape == "t4" || shape == "ti4") {
        return 4;
    }
    throw runtime_error("Unsupported shape: " + shape);
}

static ParsedQuery parse_query_file(const string& filename, const string& shape) {
    vector<string> tokens = read_query_tokens(filename);

    if (tokens.empty()) {
        throw runtime_error("Query file is empty");
    }
    if (tokens.size() % 3 != 0) {
        throw runtime_error(
            "Malformed query: expected triples of the form subject predicate object ."
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
            throw runtime_error("Only variable subjects are supported: " + atom.subject_name);
        }
        if (atom.object_name.empty() || atom.object_name[0] != '?') {
            throw runtime_error("Only variable objects are supported: " + atom.object_name);
        }

        variables.push_back(atom.subject_name);
        variables.push_back(atom.object_name);
        query.atoms.push_back(atom);
    }

    if (static_cast<int>(query.atoms.size()) != expected_atom_count(shape)) {
        throw runtime_error(
            "Shape " + shape + " expects " + to_string(expected_atom_count(shape))
            + " atoms, but the query contains " + to_string(query.atoms.size())
        );
    }

    sort(variables.begin(), variables.end());
    variables.erase(unique(variables.begin(), variables.end()), variables.end());

    query.var_names.clear();
    for (const string& name : preferred_variable_order(shape)) {
        if (binary_search(variables.begin(), variables.end(), name)) {
            query.var_names.push_back(name);
        }
    }
    for (const string& name : variables) {
        if (find(query.var_names.begin(), query.var_names.end(), name) == query.var_names.end()) {
            query.var_names.push_back(name);
        }
    }

    unordered_map<string, int> id_of_variable;
    for (int i = 0; i < static_cast<int>(query.var_names.size()); ++i) {
        id_of_variable[query.var_names[i]] = i;
    }

    for (QueryAtom& atom : query.atoms) {
        atom.subject_id = id_of_variable.at(atom.subject_name);
        atom.object_id = id_of_variable.at(atom.object_name);
    }

    return query;
}

static string predicate_to_file(const string& data_dir, const string& predicate) {
    const string property_name =
        (!predicate.empty() && predicate[0] == 'P') ? predicate : "P" + predicate;

    return data_dir + "/prop-direct-" + property_name;
}

static uint64_t next_power_of_two_at_least(uint64_t max_value) {
    uint64_t side = 1;
    while (side < max_value) {
        if (side > numeric_limits<uint64_t>::max() / 2) {
            throw overflow_error("grid_side overflow");
        }
        side <<= 1;
    }
    return side;
}

static uint64_t qdag_cardinality(const qdag& value) {
    const uint64_t height = value.Q->getHeight();
    if (height == 0) {
        return 0;
    }
    return value.Q->bv[height - 1].n_ones();
}

static LoadedQuery load_query(const Options& opt) {
    LoadedQuery loaded;
    loaded.query = parse_query_file(opt.query_file, opt.shape);

    uint64_t max_value = 0;

    for (const QueryAtom& atom : loaded.query.atoms) {
        if (loaded.relation_by_predicate.count(atom.predicate) != 0) {
            continue;
        }

        const string path = predicate_to_file(opt.data_dir, atom.predicate);
        auto relation = unique_ptr<vector<vector<uint64_t>>>(read_relation(path, 2));

        if (!relation || relation->empty()) {
            throw runtime_error("Relation file is empty or missing: " + path);
        }

        max_value = maximum_in_table(*relation, 2, max_value);
        loaded.relation_by_predicate[atom.predicate] = relation.get();
        loaded.owned_relations.push_back(move(relation));
    }

    loaded.grid_side = next_power_of_two_at_least(max_value);
    return loaded;
}

static vector<qdag> build_fresh_qdags(const LoadedQuery& loaded) {
    vector<qdag> qdags;
    qdags.reserve(loaded.query.atoms.size());

    for (const QueryAtom& atom : loaded.query.atoms) {
        qdag::att_set attributes;
        attributes.push_back(static_cast<uint64_t>(atom.subject_id));
        attributes.push_back(static_cast<uint64_t>(atom.object_id));

        // The low-arity qdag constructor may reorder its input table.
        // Give every atom a private copy so repeated predicates cannot interfere.
        vector<vector<uint64_t>> relation_copy =
            *loaded.relation_by_predicate.at(atom.predicate);

        qdags.emplace_back(
            relation_copy,
            attributes,
            loaded.grid_side,
            2,
            static_cast<uint8_t>(attributes.size())
        );
    }

    return qdags;
}

static vector<qdag> reorder_by_cardinality(const vector<qdag>& qdags) {
    vector<size_t> order(qdags.size());
    iota(order.begin(), order.end(), 0);

    stable_sort(order.begin(), order.end(), [&](size_t left, size_t right) {
        return qdag_cardinality(qdags[left]) < qdag_cardinality(qdags[right]);
    });

    vector<qdag> sorted;
    sorted.reserve(qdags.size());
    for (size_t index : order) {
        sorted.push_back(qdags[index]);
    }
    return sorted;
}

static bool shape_sorts_relations(const string& shape) {
    // Matches queries/t3_ghd.cpp and queries/ti3_ghd.cpp.
    return shape == "t3" || shape == "ti3";
}

static vector<HardcodedPlan> three_relation_plans() {
    return {
        {1, "root=0|child=1+2", {0}, {{1, 2}}},
        {2, "root=1|child=0+2", {1}, {{0, 2}}},
        {3, "root=2|child=0+1", {2}, {{0, 1}}},
        {4, "root=0+1|child=0+2", {0, 1}, {{0, 2}}},
        {5, "root=1+0|child=1+2", {1, 0}, {{1, 2}}},
        {6, "root=2+0|child=2+1", {2, 0}, {{2, 1}}},
        {7, "root=0|children=1;2", {0}, {{1}, {2}}},
        {8, "root=1|children=0;2", {1}, {{0}, {2}}},
        {9, "root=2|children=1;0", {2}, {{1}, {0}}}
    };
}

static vector<HardcodedPlan> j4_plans() {
    return {
        {1, "root=0+1|child=2+3", {0, 1}, {{2, 3}}},
        {2, "root=0+2|child=1+3", {0, 2}, {{1, 3}}},
        {3, "root=0+3|child=1+2", {0, 3}, {{1, 2}}},
        {4, "root=0|child=1+2+3", {0}, {{1, 2, 3}}},
        {5, "root=1|child=0+2+3", {1}, {{0, 2, 3}}},
        {6, "root=2|child=1+0+3", {2}, {{1, 0, 3}}},
        {7, "root=3|child=1+2+0", {3}, {{1, 2, 0}}}
    };
}

static vector<HardcodedPlan> t4_ti4_plans() {
    return {
        {1, "root=0+2|child=1+3", {0, 2}, {{1, 3}}},
        {2, "root=0+1|child=2+3", {0, 1}, {{2, 3}}},
        {3, "root=0+3|child=2+1", {0, 3}, {{2, 1}}},
        {4, "root=0|child=1+2+3", {0}, {{1, 2, 3}}},
        {5, "root=1|child=0+2+3", {1}, {{0, 2, 3}}},
        {6, "root=2|child=1+0+3", {2}, {{1, 0, 3}}},
        {7, "root=3|child=1+2+0", {3}, {{1, 2, 0}}}
    };
}

static vector<HardcodedPlan> plans_for_shape(const string& shape) {
    if (shape == "j3" || shape == "t3" || shape == "ti3") {
        return three_relation_plans();
    }
    if (shape == "j4") {
        return j4_plans();
    }
    if (shape == "t4" || shape == "ti4") {
        return t4_ti4_plans();
    }
    throw runtime_error("Unsupported shape: " + shape);
}

static const HardcodedPlan& select_plan(
    const vector<HardcodedPlan>& plans,
    int decomposition_id
) {
    for (const HardcodedPlan& plan : plans) {
        if (plan.id == decomposition_id) {
            return plan;
        }
    }
    throw runtime_error("Invalid decomposition id: " + to_string(decomposition_id));
}

static vector<qdag> select_qdags(
    const vector<qdag>& qdags,
    const vector<int>& indices
) {
    vector<qdag> selected;
    selected.reserve(indices.size());

    for (int index : indices) {
        if (index < 0 || index >= static_cast<int>(qdags.size())) {
            throw runtime_error("Hardcoded plan contains an invalid relation index");
        }
        selected.push_back(qdags[index]);
    }

    return selected;
}

static ghd build_hardcoded_ghd(
    const vector<qdag>& qdags,
    const HardcodedPlan& plan
) {
    vector<ghd> empty_children;
    vector<ghd> children;
    children.reserve(plan.child_relations.size());

    for (const vector<int>& child_group : plan.child_relations) {
        children.emplace_back(select_qdags(qdags, child_group), empty_children);
    }

    return ghd(select_qdags(qdags, plan.root_relations), children);
}

static Timing measure_yannakakis(ghd& root, qdag*& result) {
    // medir user time
    rusage usage_start{}, usage_end{};

    getrusage(RUSAGE_SELF, &usage_start);
    const auto wall_start = steady_clock::now();

    result = yannakakis(root, {});

    const auto wall_stop = steady_clock::now();
    getrusage(RUSAGE_SELF, &usage_end);

    Timing timing;
    timing.wall_seconds = duration<double>(wall_stop - wall_start).count();
    timing.user_seconds =
        timeval_to_seconds(usage_end.ru_utime)
        - timeval_to_seconds(usage_start.ru_utime);
    timing.system_seconds =
        timeval_to_seconds(usage_end.ru_stime)
        - timeval_to_seconds(usage_start.ru_stime);

    return timing;
}

static void append_benchmark(
    const Options& opt,
    const ParsedQuery& query,
    const HardcodedPlan& plan,
    bool sorted_by_cardinality,
    const Timing& timing,
    uint64_t cardinality
) {
    bool has_content = false;
    {
        ifstream test(opt.benchmark_file);
        has_content = test.good() && test.peek() != ifstream::traits_type::eof();
    }

    ofstream out(opt.benchmark_file, ios::app);
    if (!out.good()) {
        throw runtime_error("Could not open benchmark file: " + opt.benchmark_file);
    }

    if (!has_content) {
        out
            << "shape,query_file,query_id,decomposition_id,decomposition_name,"
            << "relation_order,repetition,atoms,variables,wall_seconds,"
            << "user_seconds,system_seconds,cardinality\n";
    }

    out
        << opt.shape << ','
        << opt.query_file << ','
        << opt.query_id << ','
        << plan.id << ','
        << plan.name << ','
        << (sorted_by_cardinality ? "cardinality_ascending" : "query_atom_order") << ','
        << opt.repetition << ','
        << query.atoms.size() << ','
        << query.var_names.size() << ','
        << fixed << setprecision(9)
        << timing.wall_seconds << ','
        << timing.user_seconds << ','
        << timing.system_seconds << ','
        << cardinality << '\n';
}

int main(int argc, char** argv) {
    try {
        const Options opt = parse_options(argc, argv);
        const LoadedQuery loaded = load_query(opt);
        const vector<HardcodedPlan> plans = plans_for_shape(opt.shape);
        const HardcodedPlan& plan = select_plan(plans, opt.decomposition_id);

        vector<qdag> qdags = build_fresh_qdags(loaded);
        const bool sort_by_cardinality = shape_sorts_relations(opt.shape);

        if (sort_by_cardinality) {
            qdags = reorder_by_cardinality(qdags);
        }

        ghd root = build_hardcoded_ghd(qdags, plan);

        if (opt.debug) {
            cerr << "shape=" << opt.shape
                 << " query_id=" << opt.query_id
                 << " decomposition=" << plan.id
                 << " plan=" << plan.name
                 << " relation_order="
                 << (sort_by_cardinality ? "cardinality_ascending" : "query_atom_order")
                 << '\n';
        }

        qdag* result = nullptr;
        const Timing timing = measure_yannakakis(root, result);

        if (result == nullptr) {
            throw runtime_error("yannakakis returned a null result");
        }

        const uint64_t cardinality = qdag_cardinality(*result);

        append_benchmark(
            opt,
            loaded.query,
            plan,
            sort_by_cardinality,
            timing,
            cardinality
        );

        if (opt.debug) {
            cerr << fixed << setprecision(9)
                 << "wall=" << timing.wall_seconds
                 << " user=" << timing.user_seconds
                 << " system=" << timing.system_seconds
                 << " cardinality=" << cardinality
                 << '\n';
        }

        return 0;
    } catch (const exception& error) {
        cerr << "error: " << error.what() << '\n';
        print_usage(argv[0]);
        return 1;
    }
}
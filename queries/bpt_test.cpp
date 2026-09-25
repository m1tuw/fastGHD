#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../includes/se_quadtree.hpp"

using namespace std;
namespace fs = std::filesystem;

static vector<vector<uint64_t>> read_relation(const string& path)
{
    ifstream in(path);

    if (!in.good()) {
        throw runtime_error("Could not open: " + path);
    }

    vector<vector<uint64_t>> relation;

    uint64_t x, y;

    while (in >> x >> y) {
        relation.push_back({x, y});
    }

    return relation;
}

static uint64_t base_quadtree_bytes(se_quadtree& tree)
{
    uint64_t bytes = 0;

    for (uint16_t level = 0; level < tree.getHeight(); ++level) {
        bytes += tree.bv[level].size_in_bytes();
    }

    // total_ones contiene un uint64_t por nivel
    bytes += tree.getHeight() * sizeof(uint64_t);

    return bytes;
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <data_dir>\n";
        cerr << "Example: " << argv[0] << " data3\n";
        return 1;
    }

    string data_dir = argv[1];

    if (!fs::is_directory(data_dir)) {
        cerr << "Not a directory: " << data_dir << '\n';
        return 1;
    }

    vector<string> files;

    uint64_t total_triples = 0;
    uint64_t max_id = 0;

    // --------------------------------------------------
    // Primera pasada:
    // contar triples y encontrar el máximo ID global.
    // --------------------------------------------------
    for (const auto& entry : fs::directory_iterator(data_dir)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        string name = entry.path().filename().string();

        // Es el formato que usa query_processor.cpp.
        if (name.rfind("prop-direct-P", 0) != 0) {
            continue;
        }

        string path = entry.path().string();

        ifstream in(path);

        if (!in.good()) {
            cerr << "Could not open: " << path << '\n';
            return 1;
        }

        uint64_t x, y;

        while (in >> x >> y) {
            ++total_triples;

            max_id = max(max_id, x);
            max_id = max(max_id, y);
        }

        files.push_back(path);
    }

    if (files.empty()) {
        cerr << "No prop-direct-P* files found in " << data_dir << '\n';
        return 1;
    }

    sort(files.begin(), files.end());

    // Debe ser estrictamente mayor al ID máximo,
    // porque las coordenadas son [0, grid_side).
    uint64_t grid_side = 1;

    while (grid_side <= max_id) {
        grid_side <<= 1;
    }

    cout << "relations: " << files.size() << '\n';
    cout << "triples: " << total_triples << '\n';
    cout << "max_id: " << max_id << '\n';
    cout << "grid_side: " << grid_side << '\n';
    cout << '\n';

    uint64_t total_bytes = 0;

    // --------------------------------------------------
    // Segunda pasada:
    // construir cada quadtree con el mismo grid_side.
    //
    // Se usa fork para que al terminar cada relación
    // el SO libere toda la memoria del quadtree.
    // --------------------------------------------------
    for (size_t i = 0; i < files.size(); ++i) {
        int pipefd[2];

        if (pipe(pipefd) != 0) {
            cerr << "pipe failed\n";
            return 1;
        }

        pid_t pid = fork();

        if (pid < 0) {
            cerr << "fork failed\n";
            return 1;
        }

        if (pid == 0) {
            close(pipefd[0]);

            vector<vector<uint64_t>> relation =
                read_relation(files[i]);

            if (relation.empty()) {
                _exit(1);
            }

            se_quadtree tree(
                relation,
                grid_side,
                2,
                2
            );

            uint64_t bytes =
                base_quadtree_bytes(tree);

            write(
                pipefd[1],
                &bytes,
                sizeof(bytes)
            );

            close(pipefd[1]);

            _exit(0);
        }

        close(pipefd[1]);

        uint64_t bytes = 0;

        ssize_t n = read(
            pipefd[0],
            &bytes,
            sizeof(bytes)
        );

        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);

        if (n != sizeof(bytes) ||
            !WIFEXITED(status) ||
            WEXITSTATUS(status) != 0) {

            cerr << "Error processing: "
                 << files[i] << '\n';

            return 1;
        }

        total_bytes += bytes;

        cout << "[" << (i + 1)
             << "/" << files.size()
             << "] "
             << fs::path(files[i]).filename().string()
             << ": "
             << bytes
             << " bytes\n";
    }

    double bytes_per_triple =
        static_cast<double>(total_bytes)
        / static_cast<double>(total_triples);

    double bits_per_triple =
        8.0 * bytes_per_triple;

    cout << '\n';
    cout << "============================\n";
    cout << "RESULT\n";
    cout << "============================\n";

    cout << "total_triples: "
         << total_triples << '\n';

    cout << "total_quadtree_bytes: "
         << total_bytes << '\n';

    cout << fixed << setprecision(6);

    cout << "bytes_per_triple: "
         << bytes_per_triple << '\n';

    cout << "bits_per_triple: "
         << bits_per_triple << '\n';

    return 0;
}
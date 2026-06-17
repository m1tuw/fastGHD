#include <iostream>
#include <vector>
#include <utility>

#include "../../includes/ghd_solver.hpp"

using namespace std;

/*
 * this is a testing tool now
 * input has to be 1-indexed!!
 */
int main() {
    cout << "reading input" << endl;
    int n, m;
    cin >> n >> m;
    vector<pair<int,int>> edges;
    for (int i = 0; i < m; i++) {
        int a, b;
        cin >> a >> b;
        a--; b--;
        edges.emplace_back(a, b);
    }
    auto ghd = GHDSolver().solve(n,m,edges);
    cout << ghd.weight << endl;
    return 0;
}

/*
test 1:
8 13
1 2
2 4
4 1
2 3
3 5
5 2
2 7
4 7
7 6
6 4
7 5
5 8
8 7

test 2:
6 9
1 2
2 3
3 4
4 1
1 6
3 6
6 5
5 4
2 6

test 3:
8 13
1 2
2 3
3 1
3 4
4 5
5 3
2 6
6 4
4 2
6 7
4 7
3 8
5 8

test 4 (best: 183570 ms):
10 14
1 2
2 3
3 1
3 4
2 4
2 5
5 4
2 6
5 6
5 7
7 10
10 9
9 8
8 5

*/
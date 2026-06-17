#pragma once

#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>
#include <limits>

namespace simplex {

using T = double;

template <class U>
using vec = std::vector<U>;
using vd  = vec<T>;
using vvd = vec<vd>;
using vi  = vec<int>;

constexpr T eps = 1e-8;
constexpr T inf = std::numeric_limits<T>::infinity();

/* -
name = "Simplex"
[info]
description = "Solves linear maximization problems: maximize $c^T x$ subject to $A x <= b, x >= 0$."
time = "$O(N M dot \"#pivots\")$"
- */
struct LPSolver {
    int m, n;
    vi N, B;
    vvd D;

    LPSolver(const vvd& A, const vd& b, const vd& c)
        : m((int)b.size()), n((int)c.size()), N(n + 1), B(m), D(m + 2, vd(n + 2, 0)) {
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j)
                D[i][j] = A[i][j];

        for (int i = 0; i < m; ++i) {
            B[i] = n + i;
            D[i][n] = -1;
            D[i][n + 1] = b[i];
        }

        for (int j = 0; j < n; ++j) {
            N[j] = j;
            D[m][j] = -c[j];
        }

        N[n] = -1;
        D[m + 1][n] = 1;
    }

    void pivot(int r, int s) {
        T* a = D[r].data();
        T inv = 1.0 / a[s];

        for (int i = 0; i < m + 2; ++i) {
            if (i == r || std::abs(D[i][s]) <= eps) continue;
            T* b = D[i].data();
            T inv2 = b[s] * inv;
            for (int j = 0; j < n + 2; ++j)
                b[j] -= a[j] * inv2;
            b[s] = a[s] * inv2;
        }

        for (int j = 0; j < n + 2; ++j)
            if (j != s) D[r][j] *= inv;

        for (int i = 0; i < m + 2; ++i)
            if (i != r) D[i][s] *= -inv;

        D[r][s] = inv;
        std::swap(B[r], N[s]);
    }

    bool simplex(int phase) {
        int x = m + phase - 1;
        for (;;) {
            int s = -1;
            for (int j = 0; j <= n; ++j) {
                if (N[j] == -phase) continue;
                if (s == -1 ||
                    std::make_pair(D[x][j], N[j]) < std::make_pair(D[x][s], N[s])) {
                    s = j;
                }
            }

            if (s == -1 || D[x][s] >= -eps) return true;

            int r = -1;
            for (int i = 0; i < m; ++i) {
                if (D[i][s] <= eps) continue;
                if (r == -1 ||
                    std::make_pair(D[i][n + 1] / D[i][s], B[i]) <
                    std::make_pair(D[r][n + 1] / D[r][s], B[r])) {
                    r = i;
                }
            }

            if (r == -1) return false;
            pivot(r, s);
        }
    }

    T solve(vd& x) {
        x.assign(n, 0);

        if (n == 0) {
            for (int i = 0; i < m; ++i)
                if (D[i][n + 1] < -eps) return -inf;
            return 0;
        }

        if (m == 0) {
            for (int j = 0; j < n; ++j)
                if (D[0][j] < -eps) return inf;
            return 0;
        }

        int r = 0;
        for (int i = 1; i < m; ++i)
            if (D[i][n + 1] < D[r][n + 1])
                r = i;

        if (D[r][n + 1] < -eps) {
            pivot(r, n);
            if (!simplex(2) || D[m + 1][n + 1] < -eps)
                return -inf;

            for (int i = 0; i < m; ++i) {
                if (B[i] == -1) {
                    int s = 0;
                    for (int j = 1; j <= n; ++j) {
                        if (std::make_pair(D[i][j], N[j]) < std::make_pair(D[i][s], N[s]))
                            s = j;
                    }
                    pivot(i, s);
                }
            }
        }

        bool ok = simplex(1);
        for (int i = 0; i < m; ++i)
            if (B[i] < n)
                x[B[i]] = D[i][n + 1];

        return ok ? D[m][n + 1] : inf;
    }
};

} // namespace simplex
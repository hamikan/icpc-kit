#include <bits/stdc++.h>
using namespace std;
using T = long long;

T unit() { return 0; }
T update(T a, T b) { return max(a, b); }

T base() { return LLONG_MIN; }
T calc(T a, T b) { return a + b; }

template<typename S, auto unit, auto update, auto base, auto calc>
struct Matrix {
    int N;
    vector<vector<S>> m;

    Matrix() {}
    Matrix(int N) : N(N), m(N, vector<S>(N, base())) {}
    Matrix(const vector<vector<S>>& m) : N(m.size()), m(m) {}

    vector<S>& operator[](int i) { return m[i]; }
    const vector<S>& operator[](int i) const { return m[i]; }

    static Matrix I(int N) {
        Matrix res(N);
        for(int i=0; i<N; i++) res[i][i] = unit();
        return res;
    }
    
    Matrix& operator*=(const Matrix& a) {
        assert(N == a.N);
        Matrix res(N);
        for(int i=0; i<N; i++) {
            for(int k=0; k<N; k++) {
                if(m[i][k] == base()) continue;
                for(int j=0; j<N; j++) {
                    if(a[k][j] == base()) continue;
                    res[i][j] = update(res[i][j], calc(m[i][k], a[k][j]));
                }
            }
        }
        *this = res;
        return *this;
    }
    Matrix operator*(const Matrix& a) const { return Matrix(*this) *= a; }

    Matrix pow(long long k) const {
        Matrix res = I(N);
        Matrix a = *this;
        while(k > 0) {
            if(k & 1) res *= a;
            a *= a;
            k >>= 1;
        }
        return res;
    }
};

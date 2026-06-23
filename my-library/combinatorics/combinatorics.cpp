#include <bits/stdc++.h>
#include <atcoder/modint>
using namespace std;
using mint = atcoder::modint998244353;

struct Combinatorics {
    vector<mint> fac, ifac;

    Combinatorics(int N) : fac(N + 1), ifac(N + 1) {
        fac[0] = 1;
        for(int i=1; i<=N; i++) fac[i] = fac[i-1] * i;
        ifac[N] = fac[N].inv();
        for(int i=N; i>0; i--) ifac[i-1] = ifac[i] * i;
    }

    mint C(int n, int r) {
        if(r < 0 || n < r) return 0;
        return fac[n] * ifac[r] * ifac[n-r];
    }

    mint P(int n, int r) {
        if(r < 0 || n < r) return 0;
        return fac[n] * ifac[n-r];
    }

    mint H(int n, int r) {
        if(n == 0) return (r ? 0 : 1);
        return C(n + r - 1, r);
    }
};

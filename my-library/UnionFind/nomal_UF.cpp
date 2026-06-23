#include <bits/stdc++.h>
using namespace std;

struct UnionFind {
    vector<int> par;
    vector<int> siz;

    UnionFind(int N) : par(N), siz(N, 1) {
        iota(par.begin(), par.end(), 0);
    }

    void init(int N) {
        iota(par.begin(), par.end(), 0);
        siz.assign(N, 1);
    }

    int root(int x) {
        while(par[x] != x) {
            x = par[x] = par[par[x]];
        }
        return x;
    }

    bool unite(int x, int y) {
        x = root(x);
        y = root(y);
        if(x == y) return false;
        if(siz[x] < siz[y]) swap(x, y);
        siz[x] += siz[y];
        par[y] = x;
        return true;
    }

    bool same(int x, int y) {
        return root(x) == root(y);
    }
    
    int size(int x) {
        return siz[root(x)];
    }
};

#include <bits/stdc++.h>
using namespace std;

template<typename T>
struct Grid {
    int H, W;
    vector<vector<T>> g;

    Grid() {}
    Grid(int H, int W, T val = T()) : H(H), W(W), g(H, vector<T>(W, val)) {}
    Grid(const vector<vector<T>>& g) : H(g.size()), W(g[0].size()), g(g) {}
    Grid(const vector<string>& S) : H(S.size()), W(S[0].size()) {
        g.resize(H);
        for(int i=0; i<H; i++) for(int j=0; j<W; j++) g[i].push_back(S[i][j]);
    }

    const vector<T>& operator[](int i) const { return g[i]; }
    vector<T>& operator[](int i) { return g[i]; }
    const T& operator[](pair<int,int> p) const { return g[p.first][p.second]; }
    T& operator[](pair<int,int> p) { return g[p.first][p.second]; }
    
    bool in(pair<int, int> p) const { return in(p.first, p.second); }
    bool in(int r, int c) const { return 0 <= r && r < H && 0 <= c && c < W; }
    
    int id(pair<int, int> p) const { return id(p.first, p.second); }
    int id(int r, int c) const { return r * W + c; }
    
    pair<int, int> pos(int i) const { return make_pair(i / W, i % W); }

    pair<int, int> find(T val) const {
        for(int i=0; i<H; i++) for(int j=0; j<W; j++) if(g[i][j] == val) return make_pair(i, j);
        return make_pair(-1, -1);
    }
    vector<pair<int, int>> find_all(T val) const {
        vector<pair<int, int>> res;
        for(int i=0; i<H; i++) for(int j=0; j<W; j++) if(g[i][j] == val) res.emplace_back(i, j);
        return res;
    }

    vector<pair<int, int>> to(pair<int, int> p, int mode = 4) const { return to(p.first, p.second, mode); }
    vector<pair<int, int>> to(int x, int y, int mode = 4) const {
        static const int dx[] = {0, 1, 0, -1, 1, 1, -1, -1};
        static const int dy[] = {1, 0, -1, 0, 1, -1, -1, 1};
        vector<pair<int, int>> res;
        for(int t=0; t<mode; t++) {
            int nx = x + dx[t];
            int ny = y + dy[t];
            if(in(nx, ny)) res.emplace_back(nx, ny);
        }
        return res;
    }

    friend istream& operator>>(istream& is, Grid<T>& self) {
        for(int i=0; i<self.H; i++) for(int j=0; j<self.W; j++) is >> self.g[i][j];
        return is;
    }

    friend ostream& operator<<(ostream& os, const Grid<T>& self) {
        for(int i=0; i<self.H; i++) for(int j=0; j<self.W; j++) os << self.g[i][j] << " \n"[j == self.W - 1];
        return os;
    }
};

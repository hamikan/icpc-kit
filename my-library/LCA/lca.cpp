#include <bits/stdc++.h>
using namespace std;

int N;
vector<vector<int>> graph;

struct LCA {
    int log;
    vector<vector<int>> par;
    vector<int> dist;

    LCA(int root = 0) { init(root); }

    void init(int root = 0) {
        log = 1;
        while((1 << log) < N) log++;
        par.assign(log, vector<int>(N, -1));
        dist.assign(N, 0);
        stack<pair<int,int>> dfs;
        dfs.push({root, -1});
        while(!dfs.empty()) {
            auto [v, p] = dfs.top();
            dfs.pop();
            for(int to : graph[v]) {
                if(to == p) continue;
                par[0][to] = v;
                dist[to] = dist[v] + 1;
                dfs.push({to, v});
            }
        }
        for(int k=0; k+1<log; k++) {
            for(int v=0; v<N; v++) {
                if(par[k][v] < 0) par[k+1][v] = -1;
                else par[k+1][v] = par[k][par[k][v]];
            }
        }
    }

    int query(int u, int v) const {
        if(dist[u] < dist[v]) swap(u, v);
        for(int k=0; k<log; k++) {
            if(((dist[u] - dist[v]) >> k) & 1) {
                u = par[k][u];
            }
        }
        if(u == v) return u;
        for(int k=log-1; k>=0; k--) {
            if(par[k][u] != par[k][v]) {
                u = par[k][u];
                v = par[k][v];
            }
        }
        return  par[0][u];
    }

    int get_dist(int u, int v) const { return dist[u] + dist[v] - 2 * dist[query(u, v)]; }

    bool is_on_path(int u, int v, int a) const { return get_dist(u, a) + get_dist(a, v) == get_dist(u, v); }
};

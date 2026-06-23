#include <bits/stdc++.h>
using namespace std;

struct SCC {
    int N, scc_count;
    vector<vector<int>> g, rg;
    vector<int> order, id;
    vector<bool> used;

    SCC(int N) : N(N), g(N), rg(N), used(N, false), id(N, -1), scc_count(0) {}

    void add_edge(int u, int v) {
        g[u].push_back(v);
        rg[v].push_back(u);
    }
    
    void build() {
        auto dfs = [&](auto self, int v) -> void {
            used[v] = true;
            for(int u : g[v]) if(!used[u]) self(self, u);
            order.push_back(v);
        };
        for(int v=0; v<N; v++) if(!used[v]) dfs(dfs, v);
        auto rdfs = [&](auto self, int v) -> void {
            id[v] = scc_count;
            for(int u : rg[v]) if(id[u] == -1) self(self, u);
        };
        reverse(order.begin(), order.end());
        for(int v : order) if(id[v] == -1) rdfs(rdfs, v), scc_count++;
    }

    int get_id(int u) { return id[u]; }
    bool same(int u, int v) { return id[u] == id[v]; }
    int size() { return scc_count; }

    vector<vector<int>> get_groups() {
        vector<vector<int>> groups(scc_count);
        for(int i=0; i<N; i++) groups[id[i]].push_back(i);
        return groups;
    }

    vector<vector<int>> get_dag() {
        vector<vector<int>> dag(scc_count);
        for(int v=0; v<N; v++) {
            for(int u : g[v]) {
                if(same(v, u)) continue;
                dag[id[v]].push_back(id[u]);
            }
        }
        for(int i=0; i<scc_count; i++) {
            sort(dag[i].begin(), dag[i].end());
            dag[i].erase(unique(dag[i].begin(), dag[i].end()), dag[i].end());
        }
        return dag;
    }
};

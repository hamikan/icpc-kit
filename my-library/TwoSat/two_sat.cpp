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
    
    void scc() {
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

    vector<vector<int>> get_groups() {
        vector<vector<int>> groups(scc_count);
        for(int i=0; i<N; i++) groups[id[i]].push_back(i);
        return groups;
    }

    vector<vector<int>> get_dag() {
        vector<vector<int>> dag(scc_count);
        set<pair<int, int>> edges;
        for(int v=0; v<N; v++) {
            for(int u : g[v]) {
                if(id[v] == id[u]) continue;
                if(edges.contains({id[v], id[u]})) continue;
                dag[id[v]].push_back(id[u]);
                edges.insert({id[v], id[u]});
            }
        }
        return dag;
    }
};

struct TwoSat {
    int N;
    SCC scc;
    vector<bool> answer;

    TwoSat(int N) : N(N), scc(2*N), answer(N) {}

    void add_clause(int x, bool f, int y, bool g) {
        int u0 = x + (f ? 0 : N), u1 = x + (f ? N : 0);
        int v0 = y + (g ? 0 : N), v1 = y + (g ? N : 0);
        scc.add_edge(u1, v0);
        scc.add_edge(v1, u0);
    }

    void set_true(int x, bool f) {
        add_clause(x, f, x, f);
    }

    bool satisfiable() {
        scc.scc();
        for(int i=0; i<N; i++) {
            if(scc.id[i] == scc.id[i+N]) return false;
            answer[i] = scc.id[i] > scc.id[i+N];
        }
        return true;
    }

    bool val(int x) const { return answer[x]; }
};

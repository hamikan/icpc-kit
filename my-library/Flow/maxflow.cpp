#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct Edge {
    int to, rev;
    ll cap;
};

struct Dinic {
    int n;
    vector<vector<Edge>> g;
    vector<int> level, iter;

    Dinic(int n) : n(n), g(n), level(n), iter(n) {}

    void add_edge(int from, int to, ll cap) {
        g[from].push_back(Edge{to, (int)g[to].size(), cap});
        g[to].push_back(Edge{from, (int)g[from].size() - 1, 0});
    }

    void bfs(int s) {
        fill(level.begin(), level.end(), -1);
        queue<int> q;
        level[s] = 0;
        q.push(s);
        while(!q.empty()) {
            int v = q.front(); q.pop();
            for(auto &e : g[v]) {
                if(e.cap > 0 && level[e.to] < 0) {
                    level[e.to] = level[v] + 1;
                    q.push(e.to);
                }
            }
        }
    }

    ll dfs(int v, int t, ll f) {
        if(v == t) return f;
        for(int &i=iter[v]; i<(int)g[v].size(); i++) {
            Edge &e = g[v][i];
            if(e.cap > 0 && level[v] < level[e.to]) {
                ll d = dfs(e.to, t, min(f, e.cap));
                if (d > 0) {
                    e.cap -= d;
                    g[e.to][e.rev].cap += d;
                    return d;
                }
            }
        }
        return 0;
    }

    ll max_flow(int s, int t, ll limit = LLONG_MAX) {
        ll flow = 0;
        while(flow < limit) {
            bfs(s);
            if(level[t] < 0) break;
            fill(iter.begin(), iter.end(), 0);
            ll f;
            while((f = dfs(s, t, limit - flow)) > 0) {
                flow += f;
            }
        }
        return flow;
    }
};

#include <bits/stdc++.h>
using namespace std;
using ll = long long;

template<typename T = ll>
struct Graph {
    int N;
    struct Edge { int to; T w; };
    vector<vector<Edge>> g;

    Graph(int N) : N(N), g(N) {}

    void add_edge(int u, int v, T w = 1) {
        g[u].push_back({v, w});
        g[v].push_back({u, w});
    }

    void add_arc(int u, int v, T w = 1) {
        g[u].push_back({v, w});
    }

    vector<Edge>& operator[](int i) { return g[i]; }
    const vector<Edge>& operator[](int i) const { return g[i]; }

    vector<T> dijkstra(int s) const { return dijkstra(vector<int>{s}); }
    vector<T> dijkstra(const vector<int>& starts) const {
        constexpr T INF = numeric_limits<T>::max();
        vector<T> dist(N, INF);
        priority_queue<pair<T,int>, vector<pair<T,int>>, greater<>> pq;
        for(int s : starts) { dist[s] = 0; pq.push({0, s}); }
        while(!pq.empty()) {
            auto [d, v] = pq.top(); pq.pop();
            if(dist[v] < d) continue;
            for(auto [to, w] : g[v]) {
                if(dist[to] > d + w) {
                    dist[to] = d + w;
                    pq.push({dist[to], to});
                }
            }
        }
        return dist;
    }

    vector<int> bfs(int s) const { return bfs(vector<int>{s}); }
    vector<int> bfs(const vector<int>& starts) const {
        vector<int> dist(N, -1);
        queue<int> q;
        for(int s : starts) { dist[s] = 0; q.push(s); }
        while(!q.empty()) {
            int v = q.front(); q.pop();
            for(auto [to, w] : g[v]) {
                if(dist[to] != -1) continue;
                dist[to] = dist[v] + 1;
                q.push(to);
            }
        }
        return dist;
    }

    vector<T> bfs01(int s) const { return bfs01(vector<int>{s}); }
    vector<T> bfs01(const vector<int>& starts) const {
        constexpr T INF = numeric_limits<T>::max();
        vector<T> dist(N, INF);
        deque<int> dq;
        for(int s : starts) { dist[s] = 0; dq.push_back(s); }
        while(!dq.empty()) {
            int v = dq.front(); dq.pop_front();
            for(auto [to, w] : g[v]) {
                if(dist[to] > dist[v] + w) {
                    dist[to] = dist[v] + w;
                    if(w == 0) dq.push_front(to);
                    else dq.push_back(to);
                }
            }
        }
        return dist;
    }

    vector<int> dfs(int s) const {
        vector<int> ord;
        vector<bool> visited(N, false);
        stack<int> st;
        st.push(s);
        visited[s] = true;
        while(!st.empty()) {
            int v = st.top(); st.pop();
            ord.push_back(v);
            for(auto [to, w] : g[v]) {
                if(visited[to]) continue;
                visited[to] = true;
                st.push(to);
            }
        }
        return ord;
    }

    vector<T> bellman_ford(int s) const {
        constexpr T INF = numeric_limits<T>::max();
        vector<T> dist(N, INF);
        dist[s] = 0;
        for(int i=0; i<N-1; i++) {
            for(int v=0; v<N; v++) {
                if(dist[v] == INF) continue;
                for(auto [to, w] : g[v]) {
                    if(dist[to] > dist[v] + w) dist[to] = dist[v] + w;
                }
            }
        }
        // 負閉路検出: もう1周して更新があれば負閉路
        vector<bool> neg(N, false);
        for(int i=0; i<N; i++) {
            for(int v=0; v<N; v++) {
                if(dist[v] == INF) continue;
                for(auto [to, w] : g[v]) {
                    if(dist[to] > dist[v] + w) { dist[to] = dist[v] + w; neg[to] = true; }
                    if(neg[v]) neg[to] = true;
                }
            }
        }
        for(int v=0; v<N; v++) if(neg[v]) dist[v] = -INF;
        return dist;
    }

    pair<T, vector<pair<int,int>>> kruskal() const {
        struct E { int u, v; T w; };
        vector<E> edges;
        for(int u=0; u<N; u++) for(auto [v, w] : g[u]) if(u < v) edges.push_back({u, v, w});
        sort(edges.begin(), edges.end(), [](const E& a, const E& b) { return a.w < b.w; });
        // 簡易 UnionFind
        vector<int> par(N); iota(par.begin(), par.end(), 0);
        function<int(int)> find = [&](int x) { return par[x] == x ? x : par[x] = find(par[x]); };
        T cost = 0;
        vector<pair<int,int>> used;
        for(auto [u, v, w] : edges) {
            int ru = find(u), rv = find(v);
            if(ru == rv) continue;
            par[ru] = rv;
            cost += w;
            used.emplace_back(u, v);
        }
        return {cost, used};
    }
};

#include <bits/stdc++.h>
using namespace std;

struct HLD {
    int N;
    vector<vector<int>> graph;
    vector<int> size, depth, parent, head, in, out, rev;

    HLD(int N) : N(N), graph(N), size(N, 1), depth(N, 0), parent(N, -1), head(N), in(N), out(N), rev(N) {}

    void add_edge(int u, int v) {
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    void build(int root = 0) {
        parent[root] = -1;
        depth[root] = 0;
        vector<int> ord; ord.reserve(N);
        ord.push_back(root);
        vector<int> st = {root};
        while(!st.empty()) {
            int u = st.back(); st.pop_back();
            for(int v : graph[u]) {
                if(v == parent[u]) continue;
                parent[v] = u;
                depth[v] = depth[u] + 1;
                ord.push_back(v);
                st.push_back(v);
            }
        }
        for(int i=N-1; i>=0; i--) {
            int u = ord[i];
            size[u] = 1;
            if(!graph[u].empty() && graph[u][0] == parent[u]) swap(graph[u][0], graph[u].back());
            for(auto& v : graph[u]) {
                if(v == parent[u]) continue;
                size[u] += size[v];
                if(size[v] > size[graph[u][0]]) swap(v, graph[u][0]);
            }
        }
        int t = 0;
        st = {root};
        head[root] = root;
        while(!st.empty()) {
            int u = st.back(); st.pop_back();
            in[u] = t++;
            rev[in[u]] = u;
            for(int i = (int)graph[u].size() - 1; i >= 0; i--) {
                int v = graph[u][i];
                if(v == parent[u]) continue;
                head[v] = (v == graph[u][0] ? head[u] : v);
                st.push_back(v);
            }
        }
        for(int u : ord) out[u] = in[u] + size[u];
    }

    int la(int u, int k) {
        if(k < 0 || k > depth[u]) return -1;
        while(true) {
            int u_depth = in[u] - in[head[u]];
            if(k <= u_depth) return rev[in[u]-k];
            k -= u_depth + 1;
            u = parent[head[u]];
        }
    }

    int lca(int u, int v) {
        while(true) {
            if(in[u] > in[v]) swap(u, v);
            if(head[u] == head[v]) return u;
            v = parent[head[v]];
        }
    }

    int jump(int u, int v, int k) {
        int l = lca(u, v);
        int d1 = depth[u] - depth[l];
        if(k <= d1) return la(u, k);
        int d2 = depth[v] - depth[l];
        if(k <= d1 + d2) return la(v, d1 + d2 - k);
        return -1;
    }

    int get_idx(int u) {
        return in[u];
    }

    int get_edge_idx(int u, int v) {
        if (depth[u] > depth[v]) return in[u];
        return in[v];
    }

    int dist(int u, int v) {
        return depth[u] + depth[v] - 2 * depth[lca(u, v)];
    }
    
    template<typename T>
    void for_each(int u, int v, const T& f, bool edge_query = false) {
        while(true) {
            if(in[u] > in[v]) swap(u, v);
            if(head[u] == head[v]) {
                f(in[u] + edge_query, in[v] + 1);
                break;
            }
            f(in[head[v]], in[v] + 1);
            v = parent[head[v]];
        }
    }
    
    template<typename T, typename OP, typename F>
    T query(int u, int v, const T& e, const OP& op, const F& f, bool edge_query = false) {
        return query(u, v, e, op, f, f, edge_query);
    };
    template<typename T, typename OP, typename UP, typename DOWN>
    T query(int u, int v, const T& e, const OP& op, const UP& up, const DOWN& down, bool edge_query = false) {
        int top = lca(u, v);
        T res_up = e;
        T res_down = e;
        while(true) {
            if(head[u] == head[top]) {
                res_up = op(res_up, up(in[top] + edge_query, in[u] + 1));
                break;
            }
            res_up = op(res_up, up(in[head[u]], in[u] + 1));
            u = parent[head[u]];
        }
        while(v != top) {
            if(head[v] == head[top]) {
                res_down = op(down(in[top] + 1, in[v] + 1), res_down);
                break;
            }
            res_down = op(down(in[head[v]], in[v] + 1), res_down);
            v = parent[head[v]];
        }
        return op(res_up, res_down);
    }

    pair<vector<int>, vector<int>> auxiliary_tree(vector<int> vs) {
        if (vs.empty()) return {{}, {}};
        auto comp = [&](int a, int b) {
            return in[a] < in[b];
        };
        sort(vs.begin(), vs.end(), comp);
        for(int i=vs.size()-1; i>0; i--) vs.push_back(lca(vs[i], vs[i-1]));
        sort(vs.begin(), vs.end(), comp);
        vs.erase(unique(vs.begin(), vs.end()), vs.end());
        vector<int> parent_idx(vs.size(), -1), st;
        st.push_back(0);
        for(int i=1; i<vs.size(); i++) {
            while(true) {
                int u = vs[st.back()], v = vs[i];
                if(out[u] >= out[v]) break;
                st.pop_back();
            }
            parent_idx[i] = st.back();
            st.push_back(i);
        }
        return {vs, parent_idx};
    }

    pair<int, int> subtree_query(int u, bool edge_query = false) {
        return make_pair(in[u] + edge_query, out[u]);
    }
};

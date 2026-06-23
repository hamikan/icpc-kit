#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct V {
    int p;
};

struct E {
    ll w;
};

template<typename V, typename E>
struct Rerooting {
    struct DP {
        ll acc;
        DP() : acc(0) {}
        DP(ll acc) : acc(acc) {}
        DP operator+(const DP& a) const { return *this; }
        DP add_root(const V& v) const { return *this; }
        DP through(const E& e, const V& u, const V& v) const { return *this; }
    };

    int n;
    vector<vector<pair<int, E>>> graph;
    vector<V> val;
    vector<vector<DP>> dp;
    vector<DP> up, down, ans;

    Rerooting(int n = 0) : n(n), graph(n), val(n), dp(n), up(n), down(n), ans(n) {
        for(int i=0; i<n; i++) val[i] = V{i};
    }

    void add_edge(int a, int b, const E& e = E()) {
        graph[a].emplace_back(b, e);
        graph[b].emplace_back(a, e);
    }

    void init(int root = 0) {
        vector<int> par(n, -1), order;
        order.reserve(n);
        stack<int> st;
        st.push(root);
        par[root] = -2;
        while(!st.empty()) {
            int v = st.top(); st.pop();
            order.push_back(v);
            for(const auto &[to, _]: graph[v]) {
                if(par[to] != -1) continue;
                par[to] = v;
                st.push(to);
            }
        }
        
        int len = 0;

        for(int t=n-1; t>=0; t--) {
            int v = order[t];
            int deg = (int)graph[v].size();
            len = max(len, deg);
            dp[v].resize(deg);
            DP sum;
            for(int i=0; i<deg; i++) {
                const auto &[u, ew] = graph[v][i];
                if(u == par[v]) continue;
                dp[v][i] = down[u].through(ew, val[u], val[v]);
                sum = sum + dp[v][i];
            }
            down[v] = sum.add_root(val[v]);
        }

        vector<DP> pref(len + 1), suff(len + 1);
        for(int t=0; t<n; t++) {
            int v = order[t];
            int deg = (int)graph[v].size();
            for(int i=0; i<deg; i++) {
                const auto &[u, _] = graph[v][i];
                if(u == par[v]) {
                    dp[v][i] = up[v];
                    break;
                }
            }
            pref[0] = DP();
            for(int i=0; i<deg; i++) pref[i+1] = pref[i] + dp[v][i];
            suff[deg] = DP();
            for(int i=deg-1; i>=0; i--) suff[i] = dp[v][i] + suff[i+1];
            ans[v] = (pref[deg]).add_root(val[v]);
            for(int i=0; i<deg; i++) {
                const auto &[u, ew] = graph[v][i];
                if(u == par[v]) continue;
                DP ex = pref[i] + suff[i+1];
                DP tmp = ex.add_root(val[v]);
                up[u] = tmp.through(ew, val[v], val[u]);
            }
        }
    }
    
    DP operator[](int idx) const { return ans[idx]; }
};

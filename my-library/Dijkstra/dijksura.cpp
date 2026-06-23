#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using P = pair<ll, ll>;

int N;
vector<vector<P>> graph;

vector<ll> Dijkstra() {
    priority_queue<P, vector<P>, greater<P>> pq;
    vector<ll> ans(N + 1, LLONG_MAX);
    pq.push({0, 1});
    while(!pq.empty()) {
        auto [cost, now] = pq.top();
        pq.pop();
        if(ans[now] <= cost) continue;
        ans[now] = cost;
        for(auto [to, w] : graph[now]) {
            if(ans[to] <= cost + w) continue;
            pq.push({w + cost, to});
        }
    }
    for(ll& t : ans) t = (t == LLONG_MAX ? -1 : t);
    return ans;
}

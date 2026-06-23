#include <bits/stdc++.h>
using namespace std;

int N, M;
vector<vector<int>> graph;
vector<bool> used;

void dfs(int now) {
    used[now] = true;
    for(int to : graph[now]) {
        if(used[to]) continue;
        dfs(to);
    }
}

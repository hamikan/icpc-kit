#include <bits/stdc++.h>
using namespace std;

template <typename T = long long>
struct UnionFind {
    int group_count, inner_snap;
    
    vector<int> data;
    vector<T> potential;
    
    struct History {
        int x, y;
        int data_x, data_y;
        T potential_y;
    };
    stack<History> history;
    
    UnionFind(int N, bool allocate = true) {
        init(N, allocate);
    }

    void ensure_node(int x) {
        if(data[x] != INT_MAX) return;
        history.push({-1, x, 0, 0, 0});
        data[x] = -1;
        group_count++;
    }
    
    void init(int N, bool allocate = true) {
        group_count = allocate ? N : 0;
        inner_snap = 0;
        data.assign(N, allocate ? -1 : INT_MAX);
        potential.assign(N, 0);
        stack<History> new_stack;
        swap(history, new_stack);
    }
    
    int root(int x) {
        ensure_node(x);
        while(data[x] >= 0) x = data[x];
        return x;
    }
    
    bool unite(int x, int y, T w = 0) {
        int rx = root(x), ry = root(y);
        if(rx == ry) return false;
        if(data[rx] > data[ry]) {
            swap(rx, ry);
            swap(x, y);
            w = -w;
        }
        group_count--;
        history.push({rx, ry, data[rx], data[ry], potential[ry]});
        data[rx] += data[ry];
        potential[ry] = weight(x) + w - weight(y);
        data[ry] = rx;
        return true;
    }
    
    T weight(int x) {
        ensure_node(x);
        T sum = 0;
        while(data[x] >= 0) {
            sum += potential[x];
            x = data[x];
        }
        return sum;
    }
    
    bool same(int x, int y) {
        return root(x) == root(y);
    }

    int size(int x) {
        return -data[root(x)];
    }

    int groupcount() const {
        return group_count;
    }

    T diff(int x, int y) {
        return weight(x) - weight(y);
    }
    
    void snapshot() {
        inner_snap = history.size();
    }
    
    void undo() {
        if(history.empty()) return;
        auto [x, y, data_x, data_y, potential_y] = history.top();
        history.pop();
        if(x == -1) {
            data[y] = INT_MAX;
            potential[y] = 0;
            group_count--;
            return;
        }
        group_count++;
        data[x] = data_x;
        data[y] = data_y;
        potential[y] = potential_y;
    }
    
    void rollback(int state = -1) {
        if(state == -1) state = inner_snap;
        if(history.size() <= state) return;
        while(history.size() > state) undo();
    }
};

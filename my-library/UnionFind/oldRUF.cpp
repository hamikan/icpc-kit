#include <bits/stdc++.h>
using namespace std;

template <typename T, typename P = int>
class UnionFind {
private:
    int _group_count, _inner_snap;

    struct Node {
        T par;
        int size;
        P potential;
    };

    struct HistoryNode {
        T x, y;
        Node nx, ny;
    };

    unordered_map<T, Node> _nodes;
    stack<HistoryNode> _history;

    void _ensure_node(const T& x) {
        if(_nodes.find(x) == _nodes.end()) {
            _nodes[x] = {x, 1, P{}};
            _group_count++;
        }
    }

    P _weight(T x) {
        _ensure_node(x);
        P pot_sum = P{};
        while(_nodes[x].par != x) {
            pot_sum += _nodes[x].potential;
            x = _nodes[x].par;
        }
        return pot_sum;
    }

public:
    UnionFind() {
        init();
    }
    UnionFind(int N) {
        vector<T> A(N)
        for(int i=0; i<N; i++) A[i] = static_cast<T>(i);
        UnionFind(A);
    }
    UnionFind(const vector<T>& A) {
        init();
        _nodes.reverse(A.size())
        for(T a : A) _ensure_node(a);
    }
    
    void init() {
        _group_count = 0;
        _inner_snap = 0;
        _nodes.clear();
        stack<HistoryNode> new_stack;
        _history.swap(new_stack);
    }
    
    T root(T x) {
        _ensure_node(x);
        while(_nodes[x].par != x) x = _nodes[x].par;
        return x;
    }

    bool unite(T x, T y, P w = P{}) {
        T rx = root(x), ry = root(y);
        if(rx == ry) return false;
        if(_nodes[rx].size < _nodes[ry].size) {
            swap(rx, ry);
            swap(x, y);
            w = -w;
        }
        _group_count--;
        _history.push({rx, ry, _nodes[rx], _nodes[ry]});
        _nodes[rx].size += _nodes[ry].size;
        _nodes[ry].par = rx;
        _nodes[ry].potential = _weight(x) + w - _weight(y);
        return true;
    }

    bool is_same(T x, T y) {
        return root(x) == root(y);
    }

    int size(T x) {
        return _nodes[root(x)].size;
    }

    int group_count() const {
        return _group_count;
    }

    P diff(T x, T y) {
        assert(is_same(x, y) && "diff() was called for nodes in different sets.");
        return _weight(y) - _weight(x);
    }

    void snapshot() {
        _inner_snap = _history.size();
    }

    void undo() {
        if(_history.size() == 0) return;
        _group_count++;
        auto [x, y, nx, ny] = _history.top();
        _history.pop();
        _nodes[x] = nx;
        _nodes[y] = ny;
    }

    void roll_back(int state = -1) {
        if(state == -1) state = _inner_snap;
        if(state > _history.size()) return;
        while(state < _history.size()) undo();
    }
};

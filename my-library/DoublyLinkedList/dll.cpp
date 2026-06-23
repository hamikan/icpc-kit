#include <bits/stdc++.h>
using namespace std;

template<typename T>
class DoublyLinkedList {
private:
    struct Node {
        int prev, next;
        T value;
    };

    int nil;
    int siz;
    vector<Node> nodes;

    int _insert_node(int p_id, const T& value) {
        int v_id = nodes.size();
        nodes.push_back({nodes[p_id].prev, p_id, value});
        nodes[nodes[v_id].prev].next = v_id;
        nodes[p_id].prev = v_id;
        siz++;
        return v_id;
    }
    
public:
    DoublyLinkedList() {
        nodes.push_back({0, 0, T()});
        nil = 0;
        siz = 0;
    }

    void clear() {
        nodes.clear();
        nodes.push_back({0, 0, T()});
        siz = 0;
    }

    int size() const { return siz; }
    bool empty() const { return siz == 0; }

    int insert(int id, const T& value) {
        if(id < 0 || nodes.size() <= id || nodes[id].prev == -1) return -1;
        return _insert_node(id, value);
    }

    int insert_after(int id, const T& value) {
        if(id < 0 || nodes.size() <= id || nodes[id].prev == -1) return -1;
        return _insert_node(nodes[id].next, value);
    }
    
    void push_front(const T& v) { _insert_node(nodes[nil].next, v); }
    void push_back(const T& v) { _insert_node(nil, v); }
    void pop_front() { if(!empty()) erase(nodes[nil].next); }
    void pop_back() { if(!empty()) erase(nodes[nil].prev); }

    pair<bool, int> erase(int id) {
        if(id <= 0 || nodes.size() <= id || nodes[id].prev == -1) return {false, nil};
        int p = nodes[id].prev;
        int n = nodes[id].next;
        nodes[p].next = n;
        nodes[n].prev = p;
        nodes[id].prev = -1; 
        nodes[id].next = -1;
        siz--;
        return {true, n};
    }

    T get(int id) const {
        if(id < 0 || nodes.size() <= id) return T();
        return nodes[id].value;
    }

    void print() const {
        int cur = nodes[nil].next;
        while(cur != nil) {
            cout << nodes[cur].value << " \n"[nodes[cur].next == nil];
            cur = nodes[cur].next;
        }
    }
    
    vector<T> to_vector() const {
        int cur = nodes[nil].next;
        vector<T> ret;
        while(cur != nil) {
            ret.push_back(nodes[cur].value);
            cur = nodes[cur].next;
        }
        return ret;
    }

    int begin() const { return nodes[nil].next; }
    int end() const { return nil; }
    int next(int id) const { return nodes[id].next; }
    int prev(int id) const { return nodes[id].prev; }
};

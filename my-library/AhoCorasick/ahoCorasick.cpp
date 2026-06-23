#include <bits/stdc++.h>
using namespace std;

struct AhoCorasick {
    struct Node {
        int next[26];
        int fail;
        int dict_link;
        vector<int> accept;

        Node() {
            fill(next, next + 26, -1);
            fail = -1;
            dict_link = -1;
        }
    };

    vector<Node> nodes;
    vector<string> dict;
    vector<int> order;
    
    AhoCorasick() {
        nodes.emplace_back();
    }

    int add(const string& s) {
        int id = dict.size();
        dict.push_back(s);
        int now = 0;
        for(char c : s) {
            int to = c - 'a';
            if(nodes[now].next[to] == -1) {
                nodes[now].next[to] = nodes.size();
                nodes.emplace_back();
            }
            now = nodes[now].next[to];
        }
        nodes[now].accept.push_back(id);
        return now;
    }

    void build() {
        queue<int> q;
        nodes[0].fail = 0;
        for(int i=0; i<26; i++) {
            if(nodes[0].next[i] != -1) {
                nodes[nodes[0].next[i]].fail = 0;
                q.push(nodes[0].next[i]);
                order.push_back(nodes[0].next[i]);
            } else {
                nodes[0].next[i] = 0;
            }
        }
        while(!q.empty()) {
            int n = q.front();
            q.pop();
            int f = nodes[n].fail;
            if(!nodes[f].accept.empty()) nodes[n].dict_link = f;
            else nodes[n].dict_link = nodes[f].dict_link;
            for(int i=0; i<26; i++) {
                if(nodes[n].next[i] != -1) {
                    nodes[nodes[n].next[i]].fail = nodes[f].next[i];
                    q.push(nodes[n].next[i]);
                    order.push_back(nodes[n].next[i]);
                } else {
                    nodes[n].next[i] = nodes[f].next[i];
                }
            }
        }
    }
    
    vector<int> accepts(int u) const {
        vector<int> ret;
        while(u > 0) {
            for(int id : nodes[u].accept) ret.push_back(id);
            u = nodes[u].dict_link;
        }
        return ret;
    }
    
    vector<vector<int>> match(const string& s) const {
        vector<vector<int>> ret;
        int now = 0;
        for(char c : s) {
            now = nodes[now].next[c-'a'];
            ret.push_back(accepts(now));
        }
        return ret;
    }

    string get(int id) const { return dict[id]; }
    int size() const { return nodes.size(); }
};

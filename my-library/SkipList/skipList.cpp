#include <bits/stdc++.h>
using namespace std;
using ll = long long;

template <typename T>
class SkipList {
private:
    struct Node {
        T key;
        vector<Node*> forward;
        vector<int> span;

        Node(const T& k, int level) : key(k), forward(level, nullptr), span(level, 0) {}
    };

    const int MAX_LEVEL = 32;
    Node* header;
    int current_level;
    size_t node_count;
    mt19937 rand_engine;
    const bool duplicate = true;

    int random_level() {
        int lv = 1;
        while(lv < MAX_LEVEL && rand_engine() % 2 == 0) {
            lv++;
        }
        return lv;
    }

public:
    SkipList() : current_level(1), node_count(0) {
        rand_engine.seed(random_device{}());
        header = new Node(T(), MAX_LEVEL);
    }

    size_t size() const {
        return node_count;
    }

    void insert(const T& key) {
        vector<Node*> update(MAX_LEVEL);
        vector<int> rank(MAX_LEVEL);
        Node* x = header;

        for(int i=current_level-1; i>=0; i--) {
            rank[i] = (i == current_level - 1) ? 0 : rank[i + 1];
            while(x->forward[i] && x->forward[i]->key < key) {
                rank[i] += x->span[i];
                x = x->forward[i];
            }
            update[i] = x;
        }
        
        if(!duplicate &&  x->forward[0] && x->forward[0]->key == key) return;

        int new_lv = random_level();
        if(new_lv > current_level) {
            for(int i=current_level; i<new_lv; i++) {
                rank[i] = 0;
                update[i] = header;
                update[i]->span[i] = node_count;
            }
            current_level = new_lv;
        }
        Node* new_node = new Node(key, new_lv);

        for(int i=0; i<new_lv; i++) {
            new_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = new_node;

            int old_span = update[i]->span[i];
            int new_rank = rank[0] + 1;
            update[i]->span[i] = new_rank - rank[i];
            new_node->span[i] = old_span - (new_rank - rank[i]) + 1;
        }
        
        for(int i=new_lv; i<current_level; i++) {
            update[i]->span[i]++;
        }
        
        node_count++;
    }

    bool erase(const T& key) {
        vector<Node*> update(MAX_LEVEL);
        Node* x = header;
        for(int i=current_level-1; i>=0; i--) {
            while(x->forward[i] && x->forward[i]->key < key) {
                x = x->forward[i];
            }
            update[i] = x;
        }
        x = x->forward[0];

        if(!x || x->key != key) return false;

        for(int i=0; i<current_level; i++) {
            if(update[i]->forward[i] == x) {
                update[i]->span[i] += x->span[i] - 1;
                update[i]->forward[i] = x->forward[i];
            } else {
                 update[i]->span[i]--;
            }
        }
        delete x;

        while(current_level > 1 && header->forward[current_level - 1] == nullptr) {
            current_level--;
        }
        node_count--;
        return true;
    }

    T operator[](int k) const {
        if(k < 0 || k >= node_count) {
            throw out_of_range("Index out of range");
        }
        Node* x = header;
        int traversed = -1;
        for(int i=current_level-1; i>=0; i--) {
            while (x->forward[i] && (traversed + x->span[i]) <= k) {
                traversed += x->span[i];
                x = x->forward[i];
            }
        }
        return x->key;
    }
    
    bool find(const T& key) const {
        Node* x = header;
        for(int i=current_level-1; i>=0; i--) {
            while(x->forward[i] && x->forward[i]->key < key) {
                x = x->forward[i];
            }
        }
        x = x->forward[0];
        return x && x->key == key;
    }

    size_t lower_bound(const T& key) const {
        size_t rank = 0;
        Node* x = header;
        for(int i=current_level-1; i>=0; i--) {
            while(x->forward[i] && x->forward[i]->key < key) {
                rank += x->span[i];
                x = x->forward[i];
            }
        }
        return rank;
    }

    void print() const {
        Node* x = header->forward[0];
        cout << "List (size=" << size() << ", level=" << current_level << "):" << " \n"[size() == 0];
        while(x) {
            cout << x->key << " \n"[x->forward[0] == nullptr];
            x = x->forward[0];
        }
    }
};

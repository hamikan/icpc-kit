#include <bits/stdc++.h>
using namespace std;
using T = long long;

class Xorshift {
private:
    uint64_t x;
public:
    Xorshift() : x(88172645463325252ULL) {}
    uint64_t random() { return (x ^= x << 13, x ^= x >> 7, x ^= x << 17); }
};

struct Monoid {
    static constexpr T id() { return 0; }
    static T op(T a, T b) { return a + b; }
};

struct OperatorMonoid {
    static constexpr T id() { return T(); }
    static T op(T a, T b) { return a + b; }
};

struct Modifier {
    static T op(T a, T b, int sz) { return a + b * sz; }
};

class ImplicitTreap {
private:
    Xorshift rnd;
    struct Node {
        T value, acc, lazy;
        int priority, cnt;
        bool rev;
        Node *l, *r;
        Node(T value, int priority) : value(value), acc(Monoid::id()), lazy(OperatorMonoid::id()), priority(priority), cnt(1), rev(false), l(nullptr), r(nullptr) {}
    } *root = nullptr;
    using Tree = Node*;

    int cnt(Tree t) { return t ? t->cnt : 0; }

    T acc(Tree t) { return t ? t->acc : Monoid::id(); }
    
    void pushup(Tree t) {
        if(t) {
            t->cnt = 1 + cnt(t->l) + cnt(t->r);
            t->acc = Monoid::op(acc(t->l), Monoid::op(t->value, acc(t->r)));
        }
    }

    void pushdown(Tree t) {
        if(t && t->rev) {
            t->rev = false;
            swap(t->l, t->r);
            if(t->l) t->l->rev ^= 1;
            if(t->r) t->r->rev ^= 1;
        }
        if(t && t->lazy != OperatorMonoid::id()) {
            if(t->l) {
                t->l->lazy = OperatorMonoid::op(t->l->lazy, t->lazy);
                t->l->acc = Modifier::op(t->l->acc, t->lazy, cnt(t->l));
            }
            if(t->r) {
                t->r->lazy = OperatorMonoid::op(t->r->lazy, t->lazy);
                t->r->acc = Modifier::op(t->r->acc, t->lazy, cnt(t->r));
            }
            t->value = Modifier::op(t->value, t->lazy, 1);
            t->lazy = OperatorMonoid::id();
        }
        pushup(t);
    }

    void split(Tree t, int key, Tree& l, Tree& r) {
        if(!t) {
            l = r = nullptr;
            return;
        }
        pushdown(t);
        int implicit_key = cnt(t->l) + 1;
        if(key < implicit_key) {
            split(t->l, key, l, t->l);
            r = t;
        } else {
            split(t->r, key - implicit_key, t->r, r);
            l = t;
        }
        pushup(t);
    }

    void merge(Tree& t, Tree l, Tree r) {
        pushdown(l);
        pushdown(r);
        if(!l || !r) {
            t = l ? l : r;
        } else if(l->priority > r->priority) {
            merge(l->r, l->r, r);
            t = l;
        } else {
            merge(r->l, l, r->l);
            t = r;
        }
        pushup(t);
    }

    int find(Tree t, T x, int offset, bool left = true) {
        if(Monoid::op(t->acc, x) == x) {
            return -1;
        } else {
            pushdown(t);
            if(left) {
                if(t->l && Monoid::op(t->l->acc, x) != x) {
                    return find(t->l, x, offset, left);
                } else {
                    return (Monoid::op(t->value, x) != x) ? offset + cnt(t->l) : find(t->r, x, offset + cnt(t->l) + 1, left);
                }
            } else {
                if(t->r && Monoid::op(t->r->acc, x) != x) {
                    return find(t->r, x, offset + cnt(t->l) + 1, left);
                } else {
                    return (Monoid::op(t->value, x) != x) ? offset + cnt(t->l) : find(t->l, x, offset, left);
                }
            }
        }
    }

    void dump(Tree t) {
        if(!t) return;
        pushdown(t);
        dump(t->l);
        cout << t->value << " ";
        dump(t->r);
    }

    void clear(Tree t) {
        if(!t) return;
        clear(t->l);
        clear(t->r);
        delete t;
    }

public:
    ImplicitTreap() {}
    ImplicitTreap(vector<T> vec) {
        ::reverse(vec.begin(), vec.end());
        for(T v : vec) insert(0, v);
    }

    int size() { return cnt(root); }

    void insert(int pos, T x) {
        Tree t1, t2;
        split(root, pos, t1, t2);
        merge(t1, t1, new Node(x, rnd.random()));
        merge(root, t1, t2);
    }
    
    void sorted_insert(T x) {
        Tree t1, t2;
        split(root, lower_bound(x), t1, t2);
        merge(t1, t1, new Node(x, rnd.random()));
        merge(root, t1, t2);
    }

    void set(int pos, T x) {
        if(pos < 0 || size() <= pos) throw out_of_range("Index out of range: " + to_string(pos));
        Tree t1, t2, t3;
        split(root, pos, t1, t3);
        split(t3, 1, t2, t3);
        t2->value = x;
        pushup(t2);
        merge(t1, t1, t2);
        merge(root, t1, t3);
    }
    
    void apply(int l, int r, T x) {
        Tree t1, t2, t3;
        split(root, l, t1, t2);
        split(t2, r - l, t2, t3);
        if(t2) {
            t2->lazy = OperatorMonoid::op(t2->lazy, x);
            t2->acc = Modifier::op(t2->acc, x, cnt(t2));
        }
        merge(t2, t2, t3);
        merge(root, t1, t2);
    }
    
    T query(int l, int r) {
        Tree t1, t2, t3;
        split(root, l, t1, t2);
        split(t2, r - l, t2, t3);
        T res = acc(t2);
        merge(t2, t2, t3);
        merge(root, t1, t2);
        return res;
    }

    int find(int l, int r, T x, bool left = true) {
        Tree t1, t2, t3;
        split(root, r, t1, t3);
        split(t1, l, t1, t2);
        int ret = find(t2, x, l, left);
        merge(t2, t2, t3);
        merge(root, t1, t2);
        return ret;
    }

    void erase(int l, int r) {
        if(l >= r) return;
        Tree t1, t2, t3;
        split(root, l, t1, t2);
        split(t2, r - l, t2, t3);
        if(t2) clear(t2);
        merge(root, t1, t3);
    }

    void erase(int pos) { erase(pos, pos + 1); }

    void reverse(int l, int r) {
        if(l >= r) return;
        Tree t1, t2, t3;
        split(root, r, t1, t3);
        split(t1, l, t1, t2);
        if(t2) t2->rev ^= 1;
        merge(t1, t1, t2);
        merge(root, t1, t3);
    }
    
    void rotate(int l, int m, int r) {
        if(l >= r || m <= l || m >= r) return;
        Tree t1, t2, t3, t4;
        split(root, r, t1, t4);
        split(t1, m, t1, t3);
        split(t1, l, t1, t2);
        merge(t1, t1, t3);
        merge(t1, t1, t2);
        merge(root, t1, t4);
    }

    int lower_bound(T x) {
        Tree t = root;
        int pos = 0;
        while(t) {
            pushdown(t);
            if(x <= t->value) {
                t = t->l;
            } else {
                pos += cnt(t->l) + 1;
                t = t->r;
            }
        }
        return pos;
    }

    int upper_bound(T x) {
        Tree t = root;
        int pos = 0;
        while(t) {
            pushdown(t);
            if(x < t->value) {
                t = t->l;
            } else {
                pos += cnt(t->l) + 1;
                t = t->r;
            }
        }
        return pos;
    }

    T operator[](int pos) {
        if(pos < 0 || size() <= pos) throw out_of_range("Index out of range: " + to_string(pos));
        Tree t1, t2, t3;
        split(root, pos + 1, t1, t3);
        split(t1, pos, t1, t2);
        T ret = t2->acc;
        merge(t1, t1, t2);
        merge(root, t1, t3);
        return ret;
    }

    void debug() {
        dump(root);
        cout << endl;
    }
    
    void clear() {
        clear(root);
        root = nullptr;
    }
};

#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct S {
    ll acc;
    int size;
    S operator+(const S& x) const { return {acc + x.acc, size}; }
};

struct F {
    ll add;
    bool operator==(const F& other) const { return add == other.add; }
};

S op(S a, S b) { return {a.acc + b.acc, a.size + b.size}; }

S e() { return {0, 0}; }

S mapping(F f, S x) { return {x.acc + f.add * x.size, x.size}; }

F composition(F f, F g) { return {f.add + g.add}; }

F id() { return {0}; }

template<class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
struct PersistentLazySegTree {
    struct Node {
        S data;
        F lazy;
        int l, r;
    };

    int N, size, log;
    vector<Node> d;
    vector<int> roots;
    
    explicit PersistentLazySegTree(int n, const S& x) : PersistentLazySegTree(vector<S>(n, x)) {}
    explicit PersistentLazySegTree(const vector<S>& v) : N(v.size()), size(1), log(0) {
        while(size < N) size <<= 1, log++;
        d.reserve(2 * size - 1);
        build(v);
    }

    int build(const vector<S>& v) {
        roots.push_back(build_dfs(v, 0, size));
        return latest_version();
    }

    void reserve_nodes(int q) { d.reserve(d.size() + q * (4 * log + 5)); }
    int version_count() const { return roots.size(); }
    int latest_version() const { return roots.size() - 1; }

    int set(int p, const S& x) { return set(latest_version(), p, x); }
    int set(int t, int p, const S& x) {
        roots.push_back(set_dfs(roots[t], 0, size, p, x));
        return latest_version();
    }

    int add(int p, const S& x) { return add(latest_version(), p, x); }
    int add(int t, int p, const S& x) {
        roots.push_back(add_dfs(roots[t], 0, size, p, x));
        return latest_version();
    }
    
    S get(int p) const { return get(latest_version(), p); }
    S get(int t, int p) const { return get_dfs(roots[t], 0, size, p, id()); }

    S query(int l, int r) const { return query(latest_version(), l, r); }
    S query(int t, int l, int r) const { return query_dfs(roots[t], 0, size, l, r, id()); }
    S all_query() const { return all_query(latest_version()); }
    S all_query(int t) const { return d[roots[t]].data; }

    int apply(int l, int r, const F& f) { return apply(latest_version(), l, r, f); }
    int apply(int t, int l, int r, const F& f) {
        roots.push_back(apply_dfs(roots[t], 0, size, l, r, f));
        return latest_version();
    }

    int copy(int src, int l, int r) { return copy(latest_version(), src, l, r); }
    int copy(int base, int src, int l, int r) {
        roots.push_back(copy_dfs(roots[base], roots[src], 0, size, l, r));
        return latest_version();
    }
    
    template<typename G>
    int max_right(int l, G ok) const { return max_right(latest_version(), l, ok); }
    template<typename G>
    int max_right(int t, int l, G ok) const {
        if(l == N) return N;
        S acc = e();
        return min(max_right_dfs(roots[t], 0, size, l, ok, acc, id()), N);
    }

    template<typename G>
    int min_left(int r, G ok) const { return min_left(latest_version(), r, ok); }
    template<typename G>
    int min_left(int t, int r, G ok) const {
        if(r == 0) return 0;
        S acc = e();
        return min_left_dfs(roots[t], 0, size, r, ok, acc, id());
    }

private:
    int make_node(const S& x, const F& f = id(), int l = -1, int r = -1) {
        d.push_back({x, f, l, r});
        return d.size() - 1;
    }

    int clone_node(int t) {
        d.push_back(d[t]);
        return d.size() - 1;
    }

    void all_apply(int t, const F& f) {
        d[t].data = mapping(f, d[t].data);
        d[t].lazy = composition(f, d[t].lazy);
    }

    void push(int t) {
        if(d[t].l == -1 || d[t].lazy == id()) return;
        d[t].l = clone_node(d[t].l);
        d[t].r = clone_node(d[t].r);
        all_apply(d[t].l, d[t].lazy);
        all_apply(d[t].r, d[t].lazy);
        d[t].lazy = id();
    }

    int build_dfs(const vector<S>& v, int l, int r) {
        if(r - l == 1) return make_node(l < N ? v[l] : e());
        int m = (l + r) >> 1;
        int lc = build_dfs(v, l, m);
        int rc = build_dfs(v, m, r);
        return make_node(op(d[lc].data, d[rc].data), id(), lc, rc);
    }

    int set_dfs(int t, int l, int r, int p, const S& x) {
        int nt = clone_node(t);
        if(r - l == 1) {
            d[nt].data = x;
            d[nt].lazy = id();
            return nt;
        }
        push(nt);
        int m = (l + r) >> 1;
        if(p < m) d[nt].l = set_dfs(d[nt].l, l, m, p, x);
        else d[nt].r = set_dfs(d[nt].r, m, r, p, x);
        d[nt].data = op(d[d[nt].l].data, d[d[nt].r].data);
        return nt;
    }

    int add_dfs(int t, int l, int r, int p, const S& x) {
        int nt = clone_node(t);
        if(r - l == 1) {
            d[nt].data = d[nt].data + x;
            d[nt].lazy = id();
            return nt;
        }
        push(nt);
        int m = (l + r) >> 1;
        if(p < m) d[nt].l = add_dfs(d[nt].l, l, m, p, x);
        else d[nt].r = add_dfs(d[nt].r, m, r, p, x);
        d[nt].data = op(d[d[nt].l].data, d[d[nt].r].data);
        return nt;
    }
    
    S get_dfs(int t, int l, int r, int p, const F& acc) const {
        if(r - l == 1) return mapping(acc, d[t].data);
        F next = composition(acc, d[t].lazy);
        int m = (l + r) >> 1;
        if(p < m) return get_dfs(d[t].l, l, m, p, next);
        return get_dfs(d[t].r, m, r, p, next);
    }

    S query_dfs(int t, int l, int r, int ql, int qr, const F& acc) const {
        if(qr <= l || r <= ql) return e();
        if(ql <= l && r <= qr) return mapping(acc, d[t].data);
        F next = composition(acc, d[t].lazy);
        int m = (l + r) >> 1;
        return op(query_dfs(d[t].l, l, m, ql, qr, next), query_dfs(d[t].r, m, r, ql, qr, next));
    }

    int apply_dfs(int t, int l, int r, int ql, int qr, const F& f) {
        if(qr <= l || r <= ql) return t;
        int nt = clone_node(t);
        if(ql <= l && r <= qr) {
            all_apply(nt, f);
            return nt;
        }
        push(nt);
        int m = (l + r) >> 1;
        d[nt].l = apply_dfs(d[nt].l, l, m, ql, qr, f);
        d[nt].r = apply_dfs(d[nt].r, m, r, ql, qr, f);
        d[nt].data = op(d[d[nt].l].data, d[d[nt].r].data);
        return nt;
    }

    int copy_dfs(int t, int s, int l, int r, int ql, int qr) {
        if(qr <= l || r <= ql) return t;
        if(ql <= l && r <= qr) return s;
        int nt = clone_node(t);
        int ns = clone_node(s);
        push(nt);
        push(ns);
        int m = (l + r) >> 1;
        d[nt].l = copy_dfs(d[nt].l, d[ns].l, l, m, ql, qr);
        d[nt].r = copy_dfs(d[nt].r, d[ns].r, m, r, ql, qr);
        d[nt].data = op(d[d[nt].l].data, d[d[nt].r].data);
        return nt;
    }

    template<typename G>
    int max_right_dfs(int t, int l, int r, int ql, G& ok, S& acc, const F& acc_lazy) const {
        if(r <= ql) return r;
        S cur = mapping(acc_lazy, d[t].data);
        if(ql <= l && ok(op(acc, cur))) {
            acc = op(acc, cur);
            return r;
        }
        if(r - l == 1) return l;
        F next = composition(acc_lazy, d[t].lazy);
        int m = (l + r) >> 1;
        int ret = max_right_dfs(d[t].l, l, m, ql, ok, acc, next);
        if(ret < m) return ret;
        return max_right_dfs(d[t].r, m, r, ql, ok, acc, next);
    }
    
    template<typename G>
    int min_left_dfs(int t, int l, int r, int qr, G& ok, S& acc, const F& acc_lazy) const {
        if(qr <= l) return l;
        S cur = mapping(acc_lazy, d[t].data);
        if(r <= qr && ok(op(cur, acc))) {
            acc = op(cur, acc);
            return l;
        }
        if(r - l == 1) return r;
        F next = composition(acc_lazy, d[t].lazy);
        int m = (l + r) >> 1;
        int ret = min_left_dfs(d[t].r, m, r, qr, ok, acc, next);
        if(m < ret) return ret;
        return min_left_dfs(d[t].l, l, m, qr, ok, acc, next);
    }
};

#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct S {
    ll val;
    S operator+(const S& x) const { return {val + x.val}; }
    friend ostream& operator<<(ostream& os, const S& s) {
        return os << "(" << s.val << ")";
    }
};

S op(S a, S b) { return {a.val + b.val}; }
S e() { return {0}; }

template<class S, auto op, auto e>
struct PersistentSegTree {
    struct Node {
        S data;
        int l, r;
    };

    int N, size, log;
    vector<Node> d;
    vector<int> roots;
    
    explicit PersistentSegTree(int n, const S& x = e()) : PersistentSegTree(vector<S>(n, x)) {}
    explicit PersistentSegTree(const vector<S>& v) : N(v.size()), size(1), log(0) {
        while(size < N) size <<= 1, log++;
        d.reserve(2 * size - 1);
        build(v);
    }

    int build(const vector<S>& v) {
        roots.push_back(build_dfs(v, 0, size));
        return latest_version();
    }

    void reserve_nodes(int q) { d.reserve(d.size() + q * (log + 1)); }
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
    S get(int t, int p) const { return get_dfs(roots[t], 0, size, p); }

    S query(int l, int r) const { return query(latest_version(), l, r); }
    S query(int t, int l, int r) const { return query_dfs(roots[t], 0, size, l, r); }
    S all_query() const { return all_query(latest_version()); }
    S all_query(int t) const { return d[roots[t]].data; }

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
        return min(max_right_dfs(roots[t], 0, size, l, ok, acc), N);
    }
    
    template<typename G>
    int min_left(int r, G ok) const { return min_left(latest_version(), r, ok); }
    template<typename G>
    int min_left(int t, int r, G ok) const {
        if(r == 0) return 0;
        S acc = e();
        return min_left_dfs(roots[t], 0, size, r, ok, acc);
    }

private:
    int make_node(const S& x, int l = -1, int r = -1) {
        d.push_back({x, l, r});
        return d.size() - 1;
    }
    
    int clone_node(int t) {
        d.push_back(d[t]);
        return d.size() - 1;
    }

    int build_dfs(const vector<S>& v, int l, int r) {
        if(r - l == 1) return make_node(l < N ? v[l] : e());
        int m = (l + r) >> 1;
        int lc = build_dfs(v, l, m);
        int rc = build_dfs(v, m, r);
        return make_node(op(d[lc].data, d[rc].data), lc, rc);
    }

    int set_dfs(int t, int l, int r, int p, const S& x) {
        if(r - l == 1) return make_node(x);
        int m = (l + r) >> 1;
        int lc = d[t].l, rc = d[t].r;
        if(p < m) lc = set_dfs(lc, l, m, p, x);
        else rc = set_dfs(rc, m, r, p, x);
        return make_node(op(d[lc].data, d[rc].data), lc, rc);
    }

    int add_dfs(int t, int l, int r, int p, const S& x) {
        if(r - l == 1) return make_node(d[t].data + x);
        int m = (l + r) >> 1;
        int lc = d[t].l, rc = d[t].r;
        if(p < m) lc = add_dfs(lc, l, m, p, x);
        else rc = add_dfs(rc, m, r, p, x);
        return make_node(op(d[lc].data, d[rc].data), lc, rc);
    }

    S get_dfs(int t, int l, int r, int p) const {
        if(r - l == 1) return d[t].data;
        int m = (l + r) >> 1;
        if(p < m) return get_dfs(d[t].l, l, m, p);
        return get_dfs(d[t].r, m, r, p);
    }

    S query_dfs(int t, int l, int r, int ql, int qr) const {
        if(qr <= l || r <= ql) return e();
        if(ql <= l && r <= qr) return d[t].data;
        int m = (l + r) >> 1;
        return op(query_dfs(d[t].l, l, m, ql, qr), query_dfs(d[t].r, m, r, ql, qr));
    }

    int copy_dfs(int t, int s, int l, int r, int ql, int qr) {
        if (qr <= l || r <= ql) return t;
        if (ql <= l && r <= qr) return s;
        int nt = clone_node(t);
        int m = (l + r) >> 1;
        d[nt].l = copy_dfs(d[t].l, d[s].l, l, m, ql, qr);
        d[nt].r = copy_dfs(d[t].r, d[s].r, m, r, ql, qr);
        d[nt].data = op(d[d[nt].l].data, d[d[nt].r].data);
        return nt;
    }

    template<typename G>
    int max_right_dfs(int t, int l, int r, int ql, G& ok, S& acc) const {
        if(r <= ql) return r;
        if(ql <= l) {
            S next = op(acc, d[t].data);
            if(ok(next)) {
                acc = next;
                return r;
            }
            if(r - l == 1) return l;
        }
        int m = (l + r) >> 1;
        int ret = max_right_dfs(d[t].l, l, m, ql, ok, acc);
        if(ret < m) return ret;
        return max_right_dfs(d[t].r, m, r, ql, ok, acc);
    }

    template<typename G>
    int min_left_dfs(int t, int l, int r, int qr, G& ok, S& acc) const {
        if(qr <= l) return l;
        if(r <= qr) {
            S next = op(d[t].data, acc);
            if(ok(next)) {
                acc = next;
                return l;
            }
            if(r - l == 1) return r;
        }
        int m = (l + r) >> 1;
        int ret = min_left_dfs(d[t].r, m, r, qr, ok, acc);
        if(m < ret) return ret;
        return min_left_dfs(d[t].l, l, m, qr, ok, acc);
    }
};

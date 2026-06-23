#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct RangeQuery {
    struct S {
        ll sum;
        ll min_val;
        ll max_val;
        int size;
    };

    struct F {
        ll add;
        ll update;
        bool is_update;
    };

    static constexpr ll INF_LL = 8e18;
    static constexpr S e() { return {0, INF_LL, -INF_LL, 0}; }
    static constexpr F id() { return {0, 0, false}; }

    static S op(S l, S r) {
        if(l.size == 0) return r;
        if(r.size == 0) return l;
        return {
            l.sum + r.sum,
            min(l.min_val, r.min_val),
            max(l.max_val, r.max_val),
            l.size + r.size
        };
    }

    static S mapping(F f, S x) {
        if(x.size == 0) return x;
        if(f.is_update) {
            return {
                f.update * x.size,
                f.update,
                f.update,
                x.size
            };
        } else {
            return {
                x.sum + f.add * x.size,
                x.min_val + f.add,
                x.max_val + f.add,
                x.size
            };
        }
    }

    static F composition(F f, F g) {
        if(f.is_update) {
            return f;
        } else {
            if(g.is_update) {
                return {0, g.update + f.add, true};
            } else {
                return {g.add + f.add, 0, false};
            }
        }
    }

    int N, size, log;
    vector<S> d;
    vector<F> lz;

    RangeQuery() : RangeQuery(0) {}
    explicit RangeQuery(int N) : RangeQuery(vector<ll>(N, 0)) {}
    explicit RangeQuery(const vector<ll>& v) : N(v.size()) {
        log = 32 - __builtin_clz(N - 1);
        size = 1 << log;
        d.assign(size * 2, e());
        lz.assign(size, id());
        for(int i=0; i<N; i++) d[size+i] = {v[i], v[i], v[i], 1};
        for(int i=size-1; i>=1; i--) update(i);
    }

    void set(int p, ll x) {
        assert(0 <= p && p < N);
        p += size;
        for(int i=log; i>=1; i--) push(p >> i);
        d[p] = {x, x, x, 1};
        for(int i=1; i<=log; i++) update(p >> i);
    }

    ll get(int p) {
        assert(0 <= p && p < N);
        p += size;
        for(int i=log; i>=1; i--) push(p >> i);
        return d[p].sum;
    }

    ll prod_sum(int l, int r) {
        return prod(l, r).sum;
    }
    ll prod_min(int l, int r) {
        S res = prod(l, r);
        return res.size == 0 ? INF_LL : res.min_val;
    }
    ll prod_max(int l, int r) {
        S res = prod(l, r);
        return res.size == 0 ? -INF_LL : res.max_val;
    }
    void apply_add(int l, int r, ll val) {
        apply(l, r, {val, 0, false});
    }
    void apply_update(int l, int r, ll val) {
        apply(l, r, {0, val, true});
    }

private:
    S prod(int l, int r) {
        assert(0 <= l && l <= r && r <= N);
        if (l == r) return e();
        l += size, r += size;
        for(int i=log; i>=1; i--) {
            if(((l >> i) << i) != l) push(l >> i);
            if(((r >> i) << i) != r) push((r - 1) >> i);
        }
        S sml = e(), smr = e();
        while(l < r) {
            if(l & 1) sml = op(sml, d[l++]);
            if(r & 1) smr = op(d[--r], smr);
            l >>= 1, r >>= 1;
        }
        return op(sml, smr);
    }

    void apply(int l, int r, F f) {
        assert(0 <= l && l <= r && r <= N);
        if (l == r) return;
        l += size, r += size;
        for(int i=log; i>=1; i--) {
            if(((l >> i) << i) != l) push(l >> i);
            if(((r >> i) << i) != r) push((r - 1) >> i);
        }
        int tl = l, tr = r;
        while(l < r) {
            if(l & 1) all_apply(l++, f);
            if(r & 1) all_apply(--r, f);
            l >>= 1, r >>= 1;
        }
        l = tl, r = tr;
        for(int i=1; i<=log; i++) {
            if(((l >> i) << i) != l) update(l >> i);
            if(((r >> i) << i) != r) update((r - 1) >> i);
        }
    }

    void update(int k) { d[k] = op(d[2 * k], d[2 * k + 1]); }
    void all_apply(int k, F f) {
        d[k] = mapping(f, d[k]);
        if (k < size) lz[k] = composition(f, lz[k]);
    }
    void push(int k) {
        all_apply(2 * k, lz[k]);
        all_apply(2 * k + 1, lz[k]);
        lz[k] = id();
    }
};

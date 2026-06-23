#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct S {
    ll val;
    int size;
    friend ostream& operator<<(ostream& os, const S& s) {
        return os << "(" << s.val << "," << s.size << ")";
    }
};

struct F {
    ll add;
    friend ostream& operator<<(ostream& os, const F& f) {
        return os << "(" << f.add << ")";
    }
};

S op(S a, S b) { return {a.val + b.val, a.size + b.size}; }

S e() { return {0, 0}; }

S mapping(F f, S x) { return {x.val + f.add * x.size, x.size}; }

F composition(F f, F g) { return {f.add + g.add}; }

F id() { return {0}; }

template <class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
struct LazySegTree {
    int N, size, log;
    vector<S> d;
    vector<F> lz;

    LazySegTree() : LazySegTree(0) {}
    explicit LazySegTree(int N, S v) : LazySegTree(vector<S>(N, v)) {}
    explicit LazySegTree(const vector<S>& v) : N(v.size()), log(0), size(1) {
        while ((1 << log) < N) log++, size <<= 1;
        d.assign(size << 1, e());
        lz.assign(size, id());
        for(int i=0; i<N; i++) d[size+i] = v[i];
        for(int i=size-1; i>=1; i--) update(i);
    }

    void set(int p, S x) {
        p += size;
        for(int i=log; i>=1; i--) push(p >> i);
        d[p] = x;
        for(int i=1; i<=log; i++) update(p >> i);
    }
    void add(int p, ll x) { set(p, {get(p).val + x, 1}); }

    S operator[](int p) { return get(p); }
    S get(int p) {
        p += size;
        for(int i=log; i>=1; i--) push(p >> i);
        return d[p];
    }
    
    S query(int l, int r) {
        if(l == r) return e();
        l += size, r += size;
        for(int i=log; i>=1; i--) {
            if(__builtin_ctz(l) < i) push(l >> i);
            if(__builtin_ctz(r) < i) push((r - 1) >> i);
        }
        S sml = e(), smr = e();
        while(l < r) {
            if(l & 1) sml = op(sml, d[l++]);
            if(r & 1) smr = op(d[--r], smr);
            l >>= 1, r >>= 1;
        }
        return op(sml, smr);
    }

    S all_query() { return d[1]; }

    void apply(int p, F f) {
        p += size;
        for(int i=log; i>=1; i--) push(p >> i);
        d[p] = mapping(f, d[p]);
        for(int i=1; i<=log; i++) update(p >> i);
    }
    void apply(int l, int r, F f) {
        if(l == r) return;
        l += size, r += size;
        for(int i=log; i>=1; i--) {
            if(__builtin_ctz(l) < i) push(l >> i);
            if(__builtin_ctz(r) < i) push((r - 1) >> i);
        }
        int tl = l, tr = r;
        while(l < r) {
            if(l & 1) all_apply(l++, f);
            if(r & 1) all_apply(--r, f);
            l >>= 1, r >>= 1;
        }
        l = tl, r = tr;
        for(int i=1; i<=log; i++) {
            if(__builtin_ctz(l) < i) update(l >> i);
            if(__builtin_ctz(r) < i) update((r - 1) >> i);
        }
    }

    template<typename G>
    int max_right(int l, G ok) {
        if(l == N) return N;
        l += size;
        for(int i=log; i>=1; i--) push(l >> i);
        S acc = e();
        do {
            while(~l & 1) l >>= 1;
            if(!ok(op(acc, d[l]))) {
                while(l < size) {
                    push(l);
                    l <<= 1;
                    if(ok(op(acc, d[l]))) {
                        acc = op(acc, d[l]);
                        l++;
                    }
                }
                return l - size;
            }
            acc = op(acc, d[l]);
            l++;
        } while((l & -l) != l);
        return N;
    }

    template<typename G>
    int min_left(int r, G ok) {
        if(r == 0) return 0;
        r += size;
        for(int i=log; i>=1; i--) push((r - 1) >> i);
        S acc = e();
        do {
            r--;
            while(r > 1 && (r & 1)) r >>= 1;
            if(!ok(op(d[r], acc))) {
                while(r < size) {
                    push(r);
                    r = (r << 1) + 1;
                    if(ok(op(d[r], acc))) {
                        acc = op(d[r], acc);
                        r--;
                    }
                }
                return r + 1 - size;
            }
            acc = op(d[r], acc);
        } while((r & -r) != r);
        return 0;
    }

    friend ostream& operator<<(ostream& os, LazySegTree& seg) {
        os << "[";
        for(int i=0; i<seg.N; i++) {
            os << seg.get(i);
            if(i < seg.N - 1) os << ", ";
        }
        os << "]";
        return os;
    }
    void debug() {
        for(int i=1; i<=2*size; i++) cout << d[i-1] << " \n"[(i & -i) == i];
        for(int i=1; i<=size; i++) cout << lz[i-1] << " \n"[(i & -i) == i];
    }

private:
    void update(int k) { d[k] = op(d[k<<1], d[(k<<1)+1]); }
    void all_apply(int k, F f) {
        d[k] = mapping(f, d[k]);
        if(k < size) lz[k] = composition(f, lz[k]);
    }
    void push(int k) {
        all_apply(k<<1, lz[k]);
        all_apply((k<<1)+1, lz[k]);
        lz[k] = id();
    }
};

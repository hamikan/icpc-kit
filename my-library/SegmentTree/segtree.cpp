#include <bits/stdc++.h>
using namespace std;

struct Ops {
    using T = long long;
    static constexpr T identity = 0;
    static T op(T a, T b) {return a + b;}
};

template<typename Ops>
struct SegTree {
    using T = typename Ops::T;
    int N, size, log;
    vector<T> tree;

    SegTree(int N) : SegTree(vector<T>(N, Ops::identity)) {};
    SegTree(const vector<T>& arr) : N(arr.size()), size(1), log(0) {
        while(size < N) size <<= 1, log++;
        tree.assign(2 * size, Ops::identity);
        for(int i=0; i<N; i++) tree[size+i] = arr[i];
        for(int i=size-1; i>0; i--) update(i);
    }

    void set(int p, T x) {
        p += size;
        tree[p] = x;
        for(int i=1; i<=log; i++) update(p >> i);
    }

    T operator[](int i) const {
        return tree[i+size];
    }

    T query(int l, int r) const {
        T sml = Ops::identity;
        T smr = Ops::identity;
        l += size, r += size;
        while(l < r) {
            if(l & 1) sml = Ops::op(sml, tree[l++]);
            if(r & 1) smr = Ops::op(tree[--r], smr);
            l >>= 1, r >>= 1;
        }
        return Ops::op(sml, smr);
    }

    T all_query() const {
        return tree[1];
    }

    int max_right(int l, auto ok) const {
        if(l == N) return N;
        l += size;
        T acc = Ops::identity;
        do {
            while((l & 1) == 0) l >>= 1;
            if(!ok(Ops::op(acc, tree[l]))) {
                while(l < size) {
                    l <<= 1;
                    if(ok(Ops::op(acc, tree[l]))) {
                        acc = Ops::op(acc, tree[l]);
                        l++;
                    }
                }
                return l - size;
            }
            acc = Ops::op(acc, tree[l]);
            l++;
        } while((l & -l) != l);
        return N;
    }

    int min_left(int r, auto ok) const {
        if(r == 0) return 0;
        r += size;
        T acc = Ops::identity;
        do {
            r--;
            while(r > 1 && (r & 1)) r >>= 1;
            if(!ok(Ops::op(acc, tree[r]))) {
                while(r < size) {
                    r = (r << 1) + 1;
                    if(ok(Ops::op(acc, tree[r]))) {
                        acc = Ops::op(acc, tree[r]);
                        r--;
                    }
                }
                return r + 1 - size;
            }
            acc = Ops::op(acc, tree[r]);
        } while((r & -r) != r);
        return 0;
    }

    void update(int p) {tree[p] = Ops::op(tree[p*2], tree[p*2+1]);}

    void debug() {
        for(int i=1; i<=2*size; i++) cout << tree[i-1] << " \n"[(i & -i) == i];
    }
};

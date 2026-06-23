#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct Ops {
    using T = ll;
    static T e() { return 0; }
    static T add(T a, T b) { return a + b; }
    static T sub(T a, T b) { return a - b; }
};

template<typename Ops>
class FenwickTree {
    using T = typename Ops::T;
private:
    int n;
    vector<T> bit;
    
public:
    FenwickTree(int size) : FenwickTree(size, (Ops::e())) {}
    FenwickTree(int size, T x) : FenwickTree(vector<T>(size, x)) {}
    FenwickTree(const vector<T>& v) : n(v.size()), bit(v.size() + 1, Ops::e()) {
        for(int i=1; i<=n; i++) {
            bit[i] = Ops::add(bit[i], v[i-1]);
            int p = i + (i & -i);
            if(p <= n) bit[p] = Ops::add(bit[p], bit[i]);
        }
    }

    T get(int p) const {
        return sum(p, p + 1);
    }

    void add(int i, T x) {
        for(i++; i<=n; i+=(i&-i)) bit[i] = Ops::add(bit[i], x);
    }

    void set(int i, T x) {
        add(i, Ops::sub(x, get(i)));
    }

    T sum(int i) const {
        T ret = Ops::e();
        while(i > 0) {
            ret = Ops::add(ret, bit[i]);
            i -= (i & -i);
        }
        return ret;
    }

    T sum(int l, int r) const {
        return Ops::sub(sum(r), sum(l));
    }

    int lower_bound(T x) const {
        if(x <= Ops::e()) return 0;
        int pos = 0;
        for(int k=1<<(31-__builtin_clz(n)); k>0; k>>=1) {
            if(pos + k <= n && bit[pos+k] < x) {
                x = Ops::sub(x, bit[pos+k]);
                pos += k;
            }
        }
        return pos;
    }

    int upper_bound(T x) const {
        if(x < Ops::e()) return 0;
        int pos = 0;
        for(int k=1<<(31-__builtin_clz(n)); k>0; k>>=1) {
            if(pos + k <= n && bit[pos+k] <= x) { 
                x = Ops::sub(x, bit[pos+k]);
                pos += k;
            }
        }
        return pos;
    }
};

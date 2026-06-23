#include <bits/stdc++.h>
using namespace std;
using u64 = unsigned long long;
using u128 = __uint128_t;

struct Mod61 {
    static constexpr u64 MOD = (1ULL << 61) - 1;

    u64 v;
    Mod61() : v(0) {}
    Mod61(u64 x) : v(reduce((u128)x)) {}

    static u64 norm(u64 x) { return (x >= MOD ? x - MOD : x); }
    static u64 reduce(u128 x) { return norm(norm(u64(x >> 61) + u64(x & MOD))); }

    Mod61& operator+=(const Mod61& o) { v = norm(v + o.v); return *this; }
    Mod61& operator-=(const Mod61& o) { v = norm(v - o.v + MOD); return *this; }
    Mod61& operator*=(const Mod61& o) { v = reduce((u128)v * o.v); return *this; }
    Mod61& operator/=(const Mod61& o) { return *this *= o.inv(); }

    friend Mod61 operator+(Mod61 a, const Mod61& b) { return a += b; }
    friend Mod61 operator-(Mod61 a, const Mod61& b) { return a -= b; }
    friend Mod61 operator*(Mod61 a, const Mod61& b) { return a *= b; }
    friend Mod61 operator/(Mod61 a, const Mod61& b) { return a /= b; }

    friend bool operator==(const Mod61& a, const Mod61& b) { return a.v == b.v; }
    friend bool operator!=(const Mod61& a, const Mod61& b) { return a.v != b.v; }

    static Mod61 pow(Mod61 a, u64 b) {
        Mod61 res(1);
        while(b) {
            if(b & 1) res *= a;
            a *= a;
            b >>= 1;
        }
        return res;
    }
    Mod61 inv() const { return pow(*this, MOD - 2); }

    u64 get() const { return v; }
    operator u64() const { return v; }
};

struct RollingHash {
    int n = 0;
    string s;
    Mod61 base;
    vector<Mod61> pref, power;
    
    RollingHash() = default;
    RollingHash(const string& t, u64 base_ = 0) { build(t, base_); }

    static Mod61 random_base() {
        static mt19937_64 rng((uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count());
        static uniform_int_distribution<u64> dist(1ULL << 30, Mod61::MOD - 2);
        return Mod61(dist(rng) | 1);
    }

    void build(const string& t, u64 base_ = 0) {
        s = t;
        n = s.size();
        base = (base_ ? Mod61(base_) : random_base());
        pref.assign(n + 1, Mod61(0));
        power.assign(n + 1, Mod61(1));
        for(int i=0; i<n; i++) {
            pref[i+1] = pref[i] * base + Mod61((u64)((unsigned char)(s[i]) + 1));
            power[i+1] = power[i] * base;
        }
    }

    u64 hash(int l, int r) const {
        if(l < 0 || r > n || l >= r) return 0;
        return (pref[r] - pref[l] * power[r-l]).get();
    }

    bool equal(int l1, int r1, int l2, int r2) const {
        if(r1 - l1 != r2 - l2) return false;
        return hash(l1, r1) == hash(l2, r2);
    }
    
    u64 concat(u64 h1, u64 h2, int len2) const {
        return (Mod61(h1) * power[len2] + Mod61(h2)).get();
    }

    int lcp(int i, int j) const { return lcp(i, n, j, n); }
    int lcp(int l1, int r1, int l2, int r2) const {
        int ok = 0, ng = min(r1 - l1, r2 - l2) + 1;
        while(ng - ok > 1) {
            int mid = (ok + ng) >> 1;
            if(equal(l1, l1 + mid, l2, l2 + mid)) ok = mid;
            else ng = mid;
        }
        return ok;
    }
};

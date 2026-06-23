#include <bits/stdc++.h>
using namespace std;

template<typename T>
struct ZobristHash {
private:
    vector<T> vals;
    vector<uint64_t> hash;

public:
    ZobristHash(vector<T> raw_vals) : vals(raw_vals) {
        sort(vals.begin(), vals.end());
        vals.erase(unique(vals.begin(), vals.end()), vals.end());
        mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
        hash.resize(vals.size());
        for(int i=0; i<vals.size(); i++) hash[i] = rng();
    }

    uint64_t get(T val) const {
        auto it = lower_bound(vals.begin(), vals.end(), val);
        return hash[it-vals.begin()];
    }

    int get_index(T val) const {
        return lower_bound(vals.begin(), vals.end(), val) - vals.begin();
    }

    int size() const { return hash.size(); }
};

template<typename Container>
struct Zobrist {
private:
    int n;
    vector<uint64_t> pref;

public:
    Zobrist(const Container& c, const ZobristHash<typename Container::value_type>& zh, bool is_set = true) : pref(c.size() + 1, 0) {
        if(is_set) {
            vector<bool> used(zh.size(), false);
            for(int i=0; i<c.size(); i++) {
                pref[i+1] = pref[i];
                int idx = zh.get_index(c[i]);
                if(!used[idx]) {
                    pref[i+1] += zh.get(c[i]);
                    used[idx] = true;
                }
            }
        } else {
            for(int i=0; i<c.size(); i++) {
                pref[i+1] = pref[i] + zh.get(c[i]);
            }
        }
    }

    uint64_t get(int l, int r) const {
        if(l >= r) return 0;
        return pref[r] - pref[l];
    }
};

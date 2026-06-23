#include <bits/stdc++.h>
using namespace std;

template<typename T>
struct RangeSet {
    map<T, T> mp;
    T total;
    
    RangeSet() : total(0) {}

    bool empty() const {
        return mp.empty();
    }

    bool contains(T x) const {
        auto it = mp.upper_bound(x);
        if(it == mp.begin()) return false;
        return prev(it)->first <= x && x < prev(it)->second;
    }

    bool intersects(T l, T r) const {
        if(l >= r) return false;
        auto it = mp.lower_bound(l);
        if(it != mp.end() && it->first < r) return true;
        if(it == mp.begin()) return false;
        return prev(it)->second > l;
    }

    bool covered(T l, T r) const {
        if(l >= r) return true;
        auto it = mp.upper_bound(l);
        if(it == mp.begin()) return false;
        return prev(it)->first <= l && r <= prev(it)->second;
    }
    
    void insert(T l, T r) {
        if(l >= r) return;
        auto it = mp.lower_bound(l);
        if(it != mp.begin() && l <= prev(it)->second) it = prev(it);
        while(it != mp.end() && it->first <= r) {
            l = min(l, it->first);
            r = max(r, it->second);
            total -= (it->second - it->first);
            it = mp.erase(it);
        }
        mp[l] = r;
        total += (r - l);
    }

    void erase(T l, T r) {
        if(l >= r) return;
        auto it = mp.lower_bound(l);
        if(it != mp.begin() && l < prev(it)->second) it = prev(it);
        while(it != mp.end() && it->first < r) {
            auto [L, R] = *it;
            it = mp.erase(it);
            total -= (R - L);
            if(L < l) {
                mp[L] = l;
                total += (l - L);
            }
            if(r < R) {
                mp[r] = R;
                total += (R - r);
                break;
            }
        }
    }

    T mex(T x) const {
        auto it = mp.upper_bound(x);
        if(it == mp.begin()) return x;
        return max(x, prev(it)->second);
    }

    T sum() const {
        return total;
    }

    void debug() const {
        for(auto it=mp.begin(); it!=mp.end(); it++) {
            cout << "[" << it->first << ", " << it->second << ")" << " \n"[next(it) == mp.end()];
        }
    }
    
    auto begin() const { return mp.begin(); }
    auto end() const { return mp.end(); }
};

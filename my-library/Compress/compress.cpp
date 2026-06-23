#include <bits/stdc++.h>
using namespace std;

template <typename T>
struct Compress {
    vector<T> vec;
    Compress() {} 
    Compress(const vector<T>& v) : vec(v) { build(); }
    void add(T x) { vec.push_back(x); }
    void build() {
        sort(vec.begin(), vec.end());
        vec.erase(unique(vec.begin(), vec.end()), vec.end());
    }
    int pos(T x) const {
        return lower_bound(vec.begin(), vec.end(), x) - vec.begin();
    }
    T val(int i) const {
        return vec[i];
    }
    int size() const { return vec.size(); }
};

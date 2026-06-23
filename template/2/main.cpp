#include <bits/stdc++.h>
#include <atcoder/modint>
using namespace std;
using namespace atcoder;
using ll = long long;
using ull = unsigned long long;
using ld = long double;
using mint = modint998244353;
#define rep(i, n) for (int i=0; i<n; i++)
#define test cout << "Test" << endl;
#define all(a) a.begin(), a.end()
#define rall(a) a.rbegin(), a.rend()

const ll INF = 3e18;

const int dx[4] = {1, 0, -1, 0};
const int dy[4] = {0, 1, 0, -1};

template<typename T1, typename T2>
inline bool chmax(T1& a, T2 b) {
    if(a >= b) return false;
    a = b;
    return true;
}
template<typename T1, typename T2>
inline bool chmin(T1& a, T2 b) {
    if(a <= b) return false;
    a = b;
    return true;
}

void print(ld x) { cout << fixed << setprecision(20) << x; }
void println(ld x) { cout << fixed << setprecision(20) << x << endl; }

template<typename T1, typename T2>
ostream &operator<< (ostream &os, std::pair<T1, T2> p) {
    os << "{" << p.first << ", " << p.second << "}";
    return os;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
}

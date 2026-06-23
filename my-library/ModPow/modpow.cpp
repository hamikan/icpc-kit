#include <bits/stdc++.h>
using namespace std;
using ll = long long;

ll modpow(ll a, ll b, ll m) {
    ll ret = 1;
    a = ((a % m) + m) % m;
    while(b) {
        if(b & 1) ret = ret * a % m;
        a = a * a % m;
        b >>= 1;
    }
    return ret;
}

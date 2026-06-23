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

ll division(ll a, ll b, ll m) {
    a = ((a % m) + m) % m;
    b = ((b % m) + m) % m;
    return a * modpow(b, m - 2, m) % m;
}

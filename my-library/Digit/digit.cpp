#include <bits/stdc++.h>
using namespace std;
using ll = long long;

int digit(ll x) {
    int t = 1;
    ll a = 1;
    if(ll b = a * 1e16; b <= x) { t += 16; a = b; }
    if(ll b = a * 1e8; b <= x) { t += 8; a = b; }
    if(ll b = a * 1e4; b <= x) { t += 4; a = b; }
    if(ll b = a * 1e2; b <= x) { t += 2; a = b; }
    if(ll b = a * 1e1; b <= x) { t += 1; }
    return t;
}

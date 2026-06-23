#include <bits/stdc++.h>
using namespace std;
using ll = long long;

ll extgcd(ll a, ll b, ll &x, ll &y) {
    if(b == 0) {
        x = 1; y = 0;
        return a;
    }
    ll x1, y1;
    ll g = extgcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

pair<ll,ll> crt_pair(ll r1, ll m1, ll r2, ll m2) {
    ll x, y;
    ll g = extgcd(m1, m2, x, y);
    if((r2 - r1) % g != 0) return {0, -1}; // 解なし

    __int128 lcm = (__int128)m1 / g * m2;
    __int128 t = (__int128)(r2 - r1) / g * x % (m2 / g);
    ll res = (r1 + m1 * t) % (ll)lcm;
    if(res < 0) res += lcm;
    return {res, (ll)lcm};
}

pair<ll,ll> extcrt(const vector<pair<ll,ll>>& conds) {
    ll r = conds[0].first, m = conds[0].second;
    for(size_t i=1; i<conds.size(); i++) {
        auto [nr, nm] = crt_pair(r, m, conds[i].first, conds[i].second);
        if(nm == -1) return {0, -1}; // 解なし
        r = nr;
        m = nm;
    }
    return {r, m};
}

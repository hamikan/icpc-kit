#include <bits/stdc++.h>
using namespace std;
using ll = long long;

ll extgcd(ll a, ll b, ll &x, ll &y) {
    if (b == 0) {
        x = 1; y = 0;
        return a;
    }
    ll d = extgcd(b, a % b, y, x);
    y -= (a / b) * x;
    return d;
}

int main() {
    ll a = 30, b = 18, x, y;
    ll g = extgcd(a, b, x, y);
    cout << "gcd=" << g << " x=" << x << " y=" << y << "\n";
}
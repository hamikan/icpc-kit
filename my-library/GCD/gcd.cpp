#include <bits/stdc++.h>
using namespace std;
using ll = long long;

ll gcd(ll a, ll b) {
    if(b > 0) a = gcd(b, a % b);
    return a;
}

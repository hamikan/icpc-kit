#include <bits/stdc++.h>
using namespace std;
using ll = long long;

bool check(const vector<int> &A, const vector<int> &B) {
    const int LIM = 100000;
    vector<bool> isprime(LIM + 1,true);
    isprime[0] = isprime[1] = false;
    for (int p=2; p*p<=LIM; p++) if(isprime[p]) for(int q=p*p;q<=LIM;q+=p) isprime[q]=false;
    vector<ll> primes(1, 2);
    for(int i=3; i<=LIM; i+=2) if(isprime[i]) primes.push_back(i);

    map<ll, pair<ll,ll>> mp;
    for(int i=0; i<A.size(); i++) {
        ll b = B[i];
        for(ll p : primes) {
            if(p * p > b) break;
            if(b % p) continue;
            ll pw = 1;
            while(b % p == 0) {
                b /= p;
                pw *= p;
            }
            if(mp.contains(p)) {
                ll d = min(pw, mp[p].second);
                if(mp[p].first % d != A[i] % d) return false;
            }
            if(pw > mp[p].second) mp[p] = {A[i] % pw, pw};
        }
        if(b > 1) {
            if(mp.contains(b)) {
                ll d = min(b, mp[b].second);
                if(mp[b].first % d != A[i] % d) return false;
            }
            if(b > mp[b].second) mp[b] = {A[i] % b, b};
        }
    }
    return true;
}

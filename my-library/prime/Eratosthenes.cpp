#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct Eratosthenes {
    int N;
    vector<bool> isPrime;
    vector<int> primes;
    vector<int> spf;

    explicit Eratosthenes(int N_) : N(N_), isPrime(N_ + 1, true), spf(N_ + 1, 0) {
        if(N_ < 2) return;
        isPrime[0] = isPrime[1] = false;
        for(int i=2; i<=N_; i++) {
            if(isPrime[i]) {
                spf[i] = i;
                primes.push_back(i);
            }
            for(int p : primes) {
                if((ll)i * p > N_) break;
                isPrime[i*p] = false;
                spf[i*p] = p;
                if(i % p == 0) break;
            }
        }
    }

    bool is_prime(ll n) const {
        if(n < 0) return false;
        if(n <= N) return isPrime[n];
        for(int p : primes) {
            if((ll)p * p > n) break;
            if(n % p == 0) return false;
        }
        return true;
    }

    const vector<int>& list() const {
        return primes;
    }

    int prime_count(int n) const {
        return upper_bound(primes.begin(), primes.end(), n) - primes.begin();
    }

    int divisor_count(ll n) const {
        int ret = 1;
        for(auto [p, e] : factorize(n)) ret *= (e + 1);
        return ret;
    }

    vector<pair<ll, int>> factorize(ll n) const {
        vector<pair<ll, int>> ret;
        if(n > N) {
            for(int p : primes) {
                if((ll)p * p > n) break;
                if(n % p == 0) {
                    int cnt = 0;
                    while(n % p == 0) { n /= p; cnt++; }
                    ret.emplace_back(p, cnt);
                }
            }
            if(n > 1) ret.emplace_back(n, 1);
            return ret;
        }
        while(n > 1) {
            int p = spf[n], cnt = 0;
            while(n % p == 0) { n /= p; cnt++; }
            ret.emplace_back(p, cnt);
        }
        return ret;
    }

    vector<ll> divisors(ll n) const {
        vector<ll> ret = {1};
        for(auto [p, e] : factorize(n)) {
            int sz = ret.size();
            ll pe = 1;
            for(int i=0; i<e; i++) {
                pe *= p;
                for(int j=0; j<sz; j++) ret.push_back(ret[j] * pe);
            }
        }
        sort(ret.begin(), ret.end());
        return ret;
    }

    ll euler_phi(ll n) const {
        ll ret = n;
        for(auto [p, e] : factorize(n)) ret -= ret / p;
        return ret;
    }

    int mobius(ll n) const {
        int ret = 1;
        for(auto [p, e] : factorize(n)) {
            if(e >= 2) return 0;
            ret = -ret;
        }
        return ret;
    }
};

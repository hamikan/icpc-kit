#include <bits/stdc++.h>
using namespace std;

random_device seed_gen;
mt19937_64 rnd(seed_gen());

void rndstr(int len) {
    string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", output = "";
    uniform_int_distribution<int> dist_C(0, chars.length() - 1);
    while(len--) output += chars[dist_C(rnd)];
    cout << output << endl;
}

void rndvec(int len) {
    uniform_int_distribution<int> dist_V(0, 100);
    for(int i=0; i<len; i++) cout << dist_V(rnd) << " \n"[i == len - 1];
}

int rndint(int l, int r) {
    uniform_int_distribution<int> dist_X(l, r);
    return dist_X(rnd);
}

int main(){
    int N = rndint(1, 10), T = rndint(3, 5);
    while(T--) {
        cout << N << endl;
    }
    cout << 0 << endl;
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

struct Mo {
    int N;
    vector<pair<int, int>> q;

    Mo(int N) : N(N) {}

    void add_query(int l, int r) { q.emplace_back(l, r); }

    template<typename Add, typename Del, typename Rem>
    void run(const Add add, const Del del, const Rem rem) {
        int Q = q.size(), bs = max(1, (int)(N / sqrt(Q)));
        vector<int> order(Q);
        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(), [&](int a, int b) {
            int block_a = q[a].first / bs;
            int block_b = q[b].first / bs;
            if(block_a != block_b) return block_a < block_b;
            if(block_a & 1) return q[a].second > q[b].second;
            else return q[a].second < q[b].second;
        });
        int l = 0, r = 0;
        for(int i : order) {
            auto [ql, qr] = q[i];
            while(l > ql) add(--l);
            while(r < qr) add(r++);
            while(l < ql) del(l++);
            while(r > qr) del(--r);
            rem(i);
        }
    }
};

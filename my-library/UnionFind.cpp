struct UnionFind {
    vector<int> parent_or_size;

    explicit UnionFind(int n) : parent_or_size(n, -1) {}

    int leader(int a) {
        if (parent_or_size[a] < 0) {
            return a;
        }
        return parent_or_size[a] = leader(parent_or_size[a]);
    }

    bool merge(int a, int b) {
        int x = leader(a);
        int y = leader(b);
        if (x == y) {
            return false;
        }
        if (-parent_or_size[x] < -parent_or_size[y]) {
            swap(x, y);
        }
        parent_or_size[x] += parent_or_size[y];
        parent_or_size[y] = x;
        return true;
    }

    bool same(int a, int b) {
        return leader(a) == leader(b);
    }

    int size(int a) {
        return -parent_or_size[leader(a)];
    }
};

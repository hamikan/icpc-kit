template <class S, S (*op)(S, S), S (*e)()>
struct SegTree {
    int n = 0;
    int size = 1;
    vector<S> data;

    explicit SegTree(int n_) : n(n_) {
        while (size < n) {
            size <<= 1;
        }
        data.assign(2 * size, e());
    }

    explicit SegTree(const vector<S>& values) : SegTree((int)values.size()) {
        for (int i = 0; i < n; i++) {
            data[size + i] = values[i];
        }
        for (int i = size - 1; i >= 1; i--) {
            data[i] = op(data[2 * i], data[2 * i + 1]);
        }
    }

    void set(int p, S x) {
        p += size;
        data[p] = x;
        while (p > 1) {
            p >>= 1;
            data[p] = op(data[2 * p], data[2 * p + 1]);
        }
    }

    S get(int p) const {
        return data[size + p];
    }

    S prod(int l, int r) const {
        S left = e();
        S right = e();
        l += size;
        r += size;
        while (l < r) {
            if (l & 1) {
                left = op(left, data[l++]);
            }
            if (r & 1) {
                right = op(data[--r], right);
            }
            l >>= 1;
            r >>= 1;
        }
        return op(left, right);
    }

    S all_prod() const {
        return data[1];
    }
};

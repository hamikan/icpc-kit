#include <bits/stdc++.h>
using namespace std;
using ll = long long;

template<typename T>
class BinaryTrie {
private:
    struct Node {
        array<int, 2> children;
        Node() { children.fill(-1); }
    };

    T find_xor_in_subtree(int start_node_idx, T value, int start_bit_pos, bool is_max) const {
        if(start_node_idx == -1) return -1;
        T xor_suffix = 0;
        int current_node_idx = start_node_idx;
        for(int i=start_bit_pos; i>=0; i--) {
            if(current_node_idx == -1) break;
            int value_bit = (value >> i) & 1;
            int ideal_partner = is_max ? (1 - value_bit) : value_bit;
            if(nodes_[current_node_idx].children[ideal_partner] != -1) {
                xor_suffix |= ((is_max ? T(1) : T(0)) << i);
                current_node_idx = nodes_[current_node_idx].children[ideal_partner];
            } else {
                xor_suffix |= ((is_max ? T(0) : T(1)) << i);
                current_node_idx = nodes_[current_node_idx].children[1-ideal_partner];
            }
        }
        return xor_suffix;
    }

    T find_constrained_xor(T value, T M, bool is_max) const {
        T candidate = -1;
        T current_prefix_xor = 0;
        int current_node_idx = 0;
        for(int i=BIT_LEN-1; i>=0; i--) {
            if(current_node_idx == -1) break;
            int value_bit = (value >> i) & 1;
            int M_bit = (M >> i) & 1;
            int m_path_partner = value_bit ^ M_bit;
            int alt_path_partner = 1 - m_path_partner;
            int m_path_child = nodes_[current_node_idx].children[m_path_partner];
            int alt_path_child = nodes_[current_node_idx].children[alt_path_partner];
            if((is_max ? M_bit : !M_bit) && alt_path_child != -1) {
                T prefix = current_prefix_xor | (T(is_max ? 0 : 1) << i);
                T suffix = find_xor_in_subtree(alt_path_child, value, i - 1, is_max);
                T current_candidate = prefix + suffix;
                if(candidate == -1 || (is_max ? current_candidate > candidate : current_candidate < candidate)) {
                    candidate = current_candidate;
                }
            }
            if(m_path_child != -1) {
                current_prefix_xor |= (T(M_bit) << i);
                current_node_idx = m_path_child;
            } else {
                current_node_idx = -1;
            }
        }
        if(current_node_idx != -1) {
            if(candidate == -1 || (is_max ? current_prefix_xor > candidate : current_prefix_xor < candidate)) {
                candidate = current_prefix_xor;
            }
        }
        return candidate;
    }

    vector<Node> nodes_;
    const int BIT_LEN;
public:
    BinaryTrie(int bit_len) : BIT_LEN(bit_len) { nodes_.emplace_back(); };

    void insert(T value) {
        int current_node_idx = 0;
        for(int i=BIT_LEN-1; i>=0; i--) {
            int bit = (value >> i) & 1;
            if(nodes_[current_node_idx].children[bit] == -1) {
                nodes_[current_node_idx].children[bit] = nodes_.size();
                nodes_.emplace_back();
            }
            current_node_idx = nodes_[current_node_idx].children[bit];
        }
    }
    
    T find_xor(T value, bool is_max = true) const {
        return find_xor_in_subtree(0, value, BIT_LEN - 1, is_max);
    }

    T find_xor_in_range(T value, T L, T R, bool is_max = true) const {
        if (is_max) {
            T res = find_constrained_xor(value, R, true);
            if (res != -1 && res >= L) {
                return res;
            }
        } else {
            T res = find_constrained_xor(value, L, false);
            if (res != -1 && res <= R) {
                return res;
            }
        }
        return -1;
    }
};

#include <bits/stdc++.h>
using namespace std;

template<int CHAR_SIZE = 26, char BASE_CHAR = 'a'>
class Trie {
private:
    struct Node {
        array<int, CHAR_SIZE> children;
        int count_end;
        int count_prefix;
        Node() : count_end(0), count_prefix(0) { children.fill(-1); }
    };

    string find_kth_word(int k, int current_node_idx) const {
        string result = "";
        while(true) {
            if(nodes_[current_node_idx].count_end > 0) {
                if(k <= nodes_[current_node_idx].count_end) return result;
                k -= nodes_[current_node_idx].count_end;
            }
            for(int i=0; i<CHAR_SIZE; i++) {
                int next_node_idx = nodes_[current_node_idx].children[i];
                if(next_node_idx != -1) {
                    int subtree_count = nodes_[next_node_idx].count_prefix;
                    if(k <= subtree_count) {
                        result += BASE_CHAR + i;
                        current_node_idx = next_node_idx;
                        break;
                    } else {
                        k -= subtree_count;
                    }
                }
            }
        }
    }

    void dfs(int current_node_idx, string& current_word, vector<string>& result) const {
        if(nodes_[current_node_idx].count_end > 0) for(int i=0; i<nodes_[current_node_idx].count_end; i++) result.push_back(current_word);
        for(int i=0; i<CHAR_SIZE; i++) {
            int next_node_idx = nodes_[current_node_idx].children[i];
            if(next_node_idx != -1) {
                current_word += BASE_CHAR + i;
                dfs(next_node_idx, current_word, result);
                current_word.pop_back();
            }
        }
    }

    vector<Node> nodes_;
    bool allow_duplicates_;
public:
    Trie(size_t max_nodes = 0, bool allow_duplicates = true) : allow_duplicates_(allow_duplicates) {
        if(0 < max_nodes) nodes_.reserve(max_nodes);
        nodes_.emplace_back();
    }

    void insert(const string& s) {
        if(!allow_duplicates_ && count_word(s) > 0) return;
        int current_node_idx = 0;
        nodes_[current_node_idx].count_prefix++;
        for(char c : s) {
            int char_idx = c - BASE_CHAR;
            if(nodes_[current_node_idx].children[char_idx] == -1) {
                nodes_[current_node_idx].children[char_idx] = nodes_.size();
                nodes_.emplace_back();
            }
            current_node_idx = nodes_[current_node_idx].children[char_idx];
            nodes_[current_node_idx].count_prefix++;
        }
        nodes_[current_node_idx].count_end++;
    }
    
    void erase(string_view s) {
        if(count_word(s) == 0) return;
        int current_node_idx = 0;
        nodes_[current_node_idx].count_prefix--;
        for(char c : s) {
            int char_idx = c - BASE_CHAR;
            current_node_idx = nodes_[current_node_idx].children[char_idx];
            nodes_[current_node_idx].count_prefix--;
        }
        nodes_[current_node_idx].count_end--;
    }

    void erase_prefix(string_view prefix) {
        if(prefix.empty()) {
            clear();
            return;
        }
        int current_node_idx = 0;
        for(char c : prefix) {
            int char_idx = c - BASE_CHAR;
            if(nodes_[current_node_idx].children[char_idx] == -1) return;
            current_node_idx = nodes_[current_node_idx].children[char_idx];
        }
        int count_to_erase = nodes_[current_node_idx].count_prefix;
        if(count_to_erase == 0) return;
        current_node_idx = 0;
        nodes_[current_node_idx].count_prefix -= count_to_erase;
        for(int i=0; i<prefix.size()-1; i++) {
            int char_idx = prefix[i] - BASE_CHAR;
            current_node_idx = nodes_[current_node_idx].children[char_idx];
            nodes_[current_node_idx].count_prefix -= count_to_erase;
        }
        nodes_[current_node_idx].children[prefix.back()-BASE_CHAR] = -1;
    }

    int count_word(string_view s) const {
        int current_node_idx = 0;
        for(char c : s) {
            int char_idx = c - BASE_CHAR;
            if(nodes_[current_node_idx].children[char_idx] == -1) return 0;
            current_node_idx = nodes_[current_node_idx].children[char_idx];
        }
        return nodes_[current_node_idx].count_end;
    }

    int count_prefix_matches(string_view s) const {
        int current_node_idx = 0, result = 0;
        for(char c : s) {
            int char_idx = c - BASE_CHAR;
            if(nodes_[current_node_idx].children[char_idx] == -1) break;
            current_node_idx = nodes_[current_node_idx].children[char_idx];
            if(nodes_[current_node_idx].count_end > 0) result += nodes_[current_node_idx].count_end;
        }
        return result;
    }

    int count_if_starts_with(string_view prefix) const {
        int current_node_idx = 0;
        for(char c : prefix) {
            int char_idx = c - BASE_CHAR;
            if(nodes_[current_node_idx].children[char_idx] == -1) return 0;
            current_node_idx = nodes_[current_node_idx].children[char_idx];
        }
        return nodes_[current_node_idx].count_prefix;
    }

    string find_kth_word(int k) const {
        if(k <= 0 || nodes_[0].count_prefix < k) return "";
        return find_kth_word(k, 0);
    }

    string find_kth_word_with_prefix(string_view prefix, int k) const {
        int current_node_idx = 0;
        for(char c : prefix) {
            int char_idx = c - BASE_CHAR;
            if(nodes_[current_node_idx].children[char_idx] == -1) return "";
            current_node_idx = nodes_[current_node_idx].children[char_idx];
        }
        if(k <= 0 || nodes_[current_node_idx].count_prefix < k) return "";
        return string(prefix) + find_kth_word(k, current_node_idx);
    }

    string get_lcp() const {
        string lcp = "";
        int current_node_idx = 0;
        while(true) {
            int child_count = 0, next_char_idx = -1;
            for(int i=0; i<CHAR_SIZE; i++) {
                if(nodes_[current_node_idx].children[i] != -1) {
                    child_count++;
                    next_char_idx = i;
                }
            }
            if(child_count != 1) break;
            int next_node_idx = nodes_[current_node_idx].children[next_char_idx];
            if(nodes_[next_node_idx].count_prefix != nodes_[0].count_prefix) break;
            lcp += BASE_CHAR + next_char_idx;
            current_node_idx = next_node_idx;
        }
        return lcp;
    }

    vector<string> to_vector() const {
        vector<string> result;
        string current_word;
        dfs(0, current_word, result);
        return result;
    }

    void clear() {
        nodes_.clear();
        nodes_.emplace_back();
    }

    int total_word_count() const { return nodes_[0].count_prefix; }
    
    bool empty() const { return nodes_[0].count_prefix == 0; }
};

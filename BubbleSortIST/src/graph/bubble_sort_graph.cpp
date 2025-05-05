#include "bubble_sort_graph.hpp"
#include "../utils/logging.hpp"
#include <algorithm>

BubbleSortGraph::BubbleSortGraph(int n) : n_(n) {
    // Generate all permutations
    std::vector<int> base(n);
    for (int i = 0; i < n; ++i) {
        base[i] = i + 1;
    }
    do {
        vertices_.emplace_back(base);
    } while (std::next_permutation(base.begin(), base.end()));

    // Initialize adjacency lists
    adj_lists_.resize(vertices_.size(), std::vector<std::vector<int>>(n));
    for (size_t i = 0; i < vertices_.size(); ++i) {
        for (int t = 1; t <= n - 1; ++t) {
            Permutation swapped = vertices_[i].swap(t);
            for (size_t j = 0; j < vertices_.size(); ++j) {
                if (vertices_[j] == swapped) {
                    adj_lists_[i][t - 1].push_back(j);
                    break;
                }
            }
        }
    }
    log_info("BubbleSortGraph constructed with n=" + std::to_string(n) + ", vertices=" + std::to_string(vertices_.size()));
}

void BubbleSortGraph::to_metis_format(std::vector<idx_t>& xadj, std::vector<idx_t>& adjncy) const {
    xadj.clear();
    adjncy.clear();
    xadj.push_back(0);
    for (size_t i = 0; i < vertices_.size(); ++i) {
        for (int t = 0; t < n_ - 1; ++t) {
            const auto& neighbors = adj_lists_[i][t];
            adjncy.insert(adjncy.end(), neighbors.begin(), neighbors.end());
        }
        xadj.push_back(adjncy.size());
    }
    log_info("Converted graph to METIS format: xadj size=" + std::to_string(xadj.size()) + ", adjncy size=" + std::to_string(adjncy.size()));
}
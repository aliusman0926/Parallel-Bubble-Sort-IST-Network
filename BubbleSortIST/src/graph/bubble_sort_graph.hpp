#ifndef BUBBLE_SORT_GRAPH_HPP
#define BUBBLE_SORT_GRAPH_HPP

#include "../utils/permutation.hpp"
#include <vector>
#include <metis.h>

class BubbleSortGraph {
private:
    int n_; // Dimension of the graph (B_n)
    std::vector<Permutation> vertices_; // All permutations
    std::vector<std::vector<std::vector<int>>> adj_lists_; // Adjacency lists: adj_lists_[i][t] = adjacent vertices for vertex i via swap t

public:
    BubbleSortGraph(int n);
    int num_vertices() const { return vertices_.size(); }
    const std::vector<Permutation>& vertices() const { return vertices_; }
    const std::vector<int>& get_adjacent(int v_idx, int t) const { return adj_lists_[v_idx][t]; }
    void to_metis_format(std::vector<idx_t>& xadj, std::vector<idx_t>& adjncy) const;
};

#endif
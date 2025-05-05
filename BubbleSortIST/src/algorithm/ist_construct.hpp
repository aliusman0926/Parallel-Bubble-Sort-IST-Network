#ifndef IST_CONSTRUCT_HPP
#define IST_CONSTRUCT_HPP

#include "../graph/bubble_sort_graph.hpp"
#include <vector>

int parent1(const BubbleSortGraph& graph, int v_idx, int t, int n);
std::vector<std::vector<int>> construct_ists(const BubbleSortGraph& graph, const std::vector<int>& vertices, int n);

#endif
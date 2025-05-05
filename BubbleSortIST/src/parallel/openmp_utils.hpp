#ifndef OPENMP_UTILS_HPP
#define OPENMP_UTILS_HPP

#include "../graph/bubble_sort_graph.hpp"
#include <vector>

std::vector<std::vector<int>> construct_ists_parallel(const BubbleSortGraph& graph,
                                                     const std::vector<int>& vertices, int n);

#endif
#ifndef METIS_PARTITION_HPP
#define METIS_PARTITION_HPP

#include "../graph/bubble_sort_graph.hpp"
#include <vector>
#include <metis.h>

std::vector<idx_t> partition_graph(const BubbleSortGraph& graph, int nparts);

#endif
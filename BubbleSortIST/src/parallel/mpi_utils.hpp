#ifndef MPI_UTILS_HPP
#define MPI_UTILS_HPP

#include "../graph/bubble_sort_graph.hpp"
#include <vector>
#include <mpi.h>
#include <metis.h>

std::vector<int> get_local_vertices(const std::vector<idx_t>& partition, int rank, int size, int num_vertices);
std::vector<std::vector<int>> gather_parents(const std::vector<std::vector<int>>& local_parents,
                                            const std::vector<int>& local_vertices,
                                            const std::vector<idx_t>& partition,
                                            int size, int num_vertices, int n);
void output_ists(const BubbleSortGraph& graph, const std::vector<std::vector<int>>& parents, int n,
                 const std::string& output_dir);

#endif
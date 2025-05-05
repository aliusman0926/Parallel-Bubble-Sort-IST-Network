#include "openmp_utils.hpp"
#include "../algorithm/ist_construct.hpp"
#include "../utils/logging.hpp"
#include <mpi.h>
#include <omp.h>

std::vector<std::vector<int>> construct_ists_parallel(const BubbleSortGraph& graph,
                                                     const std::vector<int>& vertices, int n) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    log_info("Rank " + std::to_string(rank) + ": Starting parallel IST construction for " + std::to_string(vertices.size()) + " vertices.");

    std::vector<std::vector<int>> parents(vertices.size(), std::vector<int>(n - 1));

    // Temporarily disable OpenMP to isolate crash
    #pragma omp parallel for
    for (size_t i = 0; i < vertices.size(); ++i) {
        int thread_id = omp_get_thread_num();
        if (vertices[i] < 0 || vertices[i] >= graph.num_vertices()) {
            log_error("Rank " + std::to_string(rank) + ": Invalid vertex index " + std::to_string(vertices[i]));
            throw std::runtime_error("Invalid vertex index");
        }
        log_info("Rank " + std::to_string(rank) + ", Thread " + std::to_string(thread_id) + ": Processing vertex " + std::to_string(vertices[i]));
        for (int t = 1; t <= n - 1; ++t) {
            parents[i][t - 1] = parent1(graph, vertices[i], t, n);
        }
    }
    log_info("Rank " + std::to_string(rank) + ": Parallel IST construction completed.");
    return parents;
}
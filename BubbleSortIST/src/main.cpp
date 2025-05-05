#include <mpi.h>
#include <iostream>
#include <filesystem>
#include "graph/bubble_sort_graph.hpp"
#include "algorithm/ist_construct.hpp"
#include "parallel/metis_partition.hpp"
#include "parallel/mpi_utils.hpp"
#include "parallel/openmp_utils.hpp"
#include "utils/logging.hpp"

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Configuration
    const int n = 3; // Dimension of B_n (test with B_4)
    const std::string output_dir = "../data/output/";

    if (rank == 0) {
        log_info("Starting IST construction for B_" + std::to_string(n) + " with " + std::to_string(size) + " MPI processes.");
        // Ensure output directory exists
        std::filesystem::create_directories(output_dir);
        log_info("Output directory " + output_dir + " ensured.");
    }

    // Create bubble-sort network
    BubbleSortGraph graph(n);
    log_info("Rank " + std::to_string(rank) + ": Graph created with " + std::to_string(graph.num_vertices()) + " vertices.");

    // Partition vertices using METIS
    std::vector<idx_t> partition = partition_graph(graph, size);
    log_info("Rank " + std::to_string(rank) + ": Partitioning completed, partition size = " + std::to_string(partition.size()));

    // Get local vertices for this process
    std::vector<int> local_vertices = get_local_vertices(partition, rank, size, graph.num_vertices());
    log_info("Rank " + std::to_string(rank) + ": Assigned " + std::to_string(local_vertices.size()) + " local vertices.");

    // Construct ISTs in parallel
    std::vector<std::vector<int>> parents = construct_ists_parallel(graph, local_vertices, n);
    log_info("Rank " + std::to_string(rank) + ": Local IST construction completed, parents size = " + std::to_string(parents.size()));

    // Gather and output results
    if (rank == 0) {
        std::vector<std::vector<int>> all_parents = gather_parents(parents, local_vertices, partition, size, graph.num_vertices(), n);
        log_info("Rank 0: Gathered all parents, size = " + std::to_string(all_parents.size()));
        output_ists(graph, all_parents, n, output_dir);
        log_info("IST construction completed. Results written to " + output_dir + "ists_B" + std::to_string(n) + ".txt");
    } else {
        gather_parents(parents, local_vertices, partition, size, graph.num_vertices(), n);
        log_info("Rank " + std::to_string(rank) + ": Gather parents completed.");
    }

    MPI_Finalize();
    return 0;
}
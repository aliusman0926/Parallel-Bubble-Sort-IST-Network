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
    const int n = 7; // Dimension of B_n (test with B_4)
    const std::string output_dir = "data/output/";

    double start_time, graph_time, partition_time, local_vertices_time, ist_time, gather_time, output_time;

    if (rank == 0) {
        start_time = MPI_Wtime();
        log_info("Starting IST construction for B_" + std::to_string(n) + " with " + std::to_string(size) + " MPI processes.");
        std::filesystem::create_directories(output_dir);
        log_info("Output directory " + output_dir + " ensured.");
    }

    // Create bubble-sort network
    BubbleSortGraph graph(n);
    if (rank == 0) {
        graph_time = MPI_Wtime();
        log_info("Rank " + std::to_string(rank) + ": Graph created with " + std::to_string(graph.num_vertices()) + " vertices.");
    }

    // Partition vertices using METIS
    std::vector<idx_t> partition = partition_graph(graph, size);
    if (rank == 0) {
        partition_time = MPI_Wtime();
        log_info("Rank " + std::to_string(rank) + ": Partitioning completed, partition size = " + std::to_string(partition.size()));
    }

    // Get local vertices for this process
    std::vector<int> local_vertices = get_local_vertices(partition, rank, size, graph.num_vertices());
    if (rank == 0) {
        local_vertices_time = MPI_Wtime();
        log_info("Rank " + std::to_string(rank) + ": Assigned " + std::to_string(local_vertices.size()) + " local vertices.");
    }

    // Construct ISTs in parallel
    std::vector<std::vector<int>> parents = construct_ists_parallel(graph, local_vertices, n);
    if (rank == 0) {
        ist_time = MPI_Wtime();
        log_info("Rank " + std::to_string(rank) + ": Local IST construction completed, parents size = " + std::to_string(parents.size()));
    }

    // Gather and output results
    if (rank == 0) {
        std::vector<std::vector<int>> all_parents = gather_parents(parents, local_vertices, partition, size, graph.num_vertices(), n);
        gather_time = MPI_Wtime();
        log_info("Rank 0: Gathered all parents, size = " + std::to_string(all_parents.size()));
        output_ists(graph, all_parents, n, output_dir);
        output_time = MPI_Wtime();
        log_info("IST construction completed. Results written to " + output_dir + "ists_B" + std::to_string(n) + ".txt");
        log_info("Timing: Graph=" + std::to_string(graph_time - start_time) +
                 "s, Partition=" + std::to_string(partition_time - graph_time) +
                 "s, LocalVertices=" + std::to_string(local_vertices_time - partition_time) +
                 "s, IST=" + std::to_string(ist_time - local_vertices_time) +
                 "s, Gather=" + std::to_string(gather_time - ist_time) +
                 "s, Output=" + std::to_string(output_time - gather_time) +
                 "s, Total=" + std::to_string(output_time - start_time) + "s");
    } else {
        gather_parents(parents, local_vertices, partition, size, graph.num_vertices(), n);
        if (rank == 0) {
            gather_time = MPI_Wtime();
            log_info("Rank " + std::to_string(rank) + ": Gather parents completed.");
        }
    }

    MPI_Finalize();
    return 0;
}
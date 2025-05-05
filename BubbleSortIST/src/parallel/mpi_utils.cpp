#include "mpi_utils.hpp"
#include "../utils/logging.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::vector<int> get_local_vertices(const std::vector<idx_t>& partition, int rank, int size, int num_vertices) {
    std::vector<int> local_vertices;
    for (int i = 0; i < num_vertices; ++i) {
        if (partition[i] == rank) {
            local_vertices.push_back(i);
        }
    }
    return local_vertices;
}

std::vector<std::vector<int>> gather_parents(const std::vector<std::vector<int>>& local_parents,
                                            const std::vector<int>& local_vertices,
                                            const std::vector<idx_t>& partition,
                                            int size, int num_vertices, int n) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    log_info("Rank " + std::to_string(rank) + ": Entering gather_parents, local_parents size = " + std::to_string(local_parents.size()));

    std::vector<std::vector<int>> all_parents(num_vertices, std::vector<int>(n - 1, -1));
    std::vector<int> counts(size), displs(size);
    int local_size = local_parents.size() * (n - 1);

    // Gather counts
    log_info("Rank " + std::to_string(rank) + ": Gathering counts, local_size = " + std::to_string(local_size));
    MPI_Gather(&local_size, 1, MPI_INT, counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        displs[0] = 0;
        for (int i = 1; i < size; ++i) {
            displs[i] = displs[i - 1] + counts[i - 1];
        }
        log_info("Rank 0: Computed displacements, displs = [" + [&displs]() {
            std::stringstream ss;
            for (int d : displs) ss << d << " ";
            return ss.str();
        }() + "]");
    }

    // Gather parents
    std::vector<int> flat_local_parents;
    for (const auto& p : local_parents) {
        flat_local_parents.insert(flat_local_parents.end(), p.begin(), p.end());
    }
    std::vector<int> flat_all_parents(num_vertices * (n - 1));
    log_info("Rank " + std::to_string(rank) + ": Calling MPI_Gatherv, flat_local_parents size = " + std::to_string(flat_local_parents.size()));
    MPI_Gatherv(flat_local_parents.data(), local_size, MPI_INT, flat_all_parents.data(),
                counts.data(), displs.data(), MPI_INT, 0, MPI_COMM_WORLD);
    log_info("Rank " + std::to_string(rank) + ": MPI_Gatherv completed.");

    // Reconstruct all_parents
    if (rank == 0) {
        for (int r = 0; r < size; ++r) {
            std::vector<int> local_verts = get_local_vertices(partition, r, size, num_vertices);
            for (size_t i = 0; i < local_verts.size(); ++i) {
                int v_idx = local_verts[i];
                for (int t = 0; t < n - 1; ++t) {
                    all_parents[v_idx][t] = flat_all_parents[displs[r] + i * (n - 1) + t];
                }
            }
        }
        log_info("Rank 0: Reconstructed all_parents.");
    }
    return all_parents;
}

void output_ists(const BubbleSortGraph& graph, const std::vector<std::vector<int>>& parents, int n,
                 const std::string& output_dir) {
    std::string filename = output_dir + "ists_B" + std::to_string(n) + ".txt";
    std::ofstream out(filename);
    if (!out.is_open()) {
        log_error("Failed to open output file: " + filename);
        throw std::runtime_error("Cannot open output file: " + filename);
    }
    for (int t = 1; t <= n - 1; ++t) {
        out << "Tree T_" << t << "^" << n << ":\n";
        for (size_t i = 0; i < parents.size(); ++i) {
            if (parents[i][t - 1] != -1) {
                out << "Vertex " << graph.vertices()[i].to_string() << " -> Parent "
                    << graph.vertices()[parents[i][t - 1]].to_string() << "\n";
            }
        }
        out << "\n";
    }
    out.close();
    log_info("Output written to " + filename);
}
#include "metis_partition.hpp"
#include "../utils/logging.hpp"
#include <metis.h>
#include <mpi.h>
#include <omp.h>
#include <stdexcept>
#include <sstream>

std::vector<idx_t> partition_graph(const BubbleSortGraph& graph, int nparts) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    log_info("Rank " + std::to_string(rank) + ": Starting METIS partitioning for " + std::to_string(graph.num_vertices()) + " vertices, nparts = " + std::to_string(nparts));

    // Handle single partition or small graphs
    if (nparts == 1 || graph.num_vertices() <= 2) {
        log_info("Rank " + std::to_string(rank) + ": Single partition or small graph, assigning all vertices to rank 0.");
        std::vector<idx_t> partition(graph.num_vertices(), 0);
        return partition;
    }

    std::vector<idx_t> adjncy, xadj;
    graph.to_metis_format(xadj, adjncy);

    // Validate xadj and adjncy
    if (xadj.size() != graph.num_vertices() + 1) {
        log_error("Invalid xadj size: " + std::to_string(xadj.size()) + ", expected " + std::to_string(graph.num_vertices() + 1));
        throw std::runtime_error("Invalid xadj size");
    }
    for (size_t i = 0; i < xadj.size() - 1; ++i) {
        if (xadj[i] > xadj[i + 1]) {
            log_error("Invalid xadj: xadj[" + std::to_string(i) + "] = " + std::to_string(xadj[i]) + " > xadj[" + std::to_string(i + 1) + "] = " + std::to_string(xadj[i + 1]));
            throw std::runtime_error("Invalid xadj array");
        }
    }
    for (idx_t v : adjncy) {
        if (v < 0 || v >= static_cast<idx_t>(graph.num_vertices())) {
            log_error("Invalid adjncy vertex: " + std::to_string(v));
            throw std::runtime_error("Invalid adjncy vertex");
        }
    }
    log_info("Rank " + std::to_string(rank) + ": xadj = [" + [&xadj]() {
        std::stringstream ss;
        for (idx_t x : xadj) ss << x << " ";
        return ss.str();
    }() + "], adjncy = [" + [&adjncy]() {
        std::stringstream ss;
        for (idx_t a : adjncy) ss << a << " ";
        return ss.str();
    }() + "]");

    idx_t nvtxs = graph.num_vertices();
    idx_t ncon = 1; // Number of balancing constraints
    idx_t objval;
    std::vector<idx_t> part(nvtxs);

    // Set METIS options
    idx_t options[METIS_NOPTIONS];
    METIS_SetDefaultOptions(options);
    options[METIS_OPTION_PTYPE] = METIS_PTYPE_KWAY;
    options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;
    options[METIS_OPTION_NUMBERING] = 0; // C-style numbering
    options[METIS_OPTION_NITER] = 10; // Number of iterations
    options[METIS_OPTION_NCUTS] = 1; // Number of different partitions to compute

    // Compute target partition weights
    std::vector<real_t> tpwgts(nparts * ncon, 1.0 / nparts);
    std::vector<real_t> ubvec(ncon, 1.05); // 5% imbalance tolerance

    log_info("Rank " + std::to_string(rank) + ": METIS parameters: nvtxs = " + std::to_string(nvtxs) +
             ", ncon = " + std::to_string(ncon) + ", nparts = " + std::to_string(nparts));

    // Call METIS
    int ret = METIS_PartGraphKway(&nvtxs, &ncon, xadj.data(), adjncy.data(), nullptr, nullptr, nullptr,
                                  &nparts, tpwgts.data(), ubvec.data(), options, &objval, part.data());
    if (ret != METIS_OK) {
        log_error("METIS_PartGraphKway failed with return code " + std::to_string(ret));
        throw std::runtime_error("METIS partitioning failed");
    }

    log_info("Rank " + std::to_string(rank) + ": METIS partitioning completed, objval = " + std::to_string(objval));
    return part;
}
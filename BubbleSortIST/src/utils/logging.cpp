#include "logging.hpp"
#include <iostream>
#include <mpi.h>

void log_info(const std::string& message) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::cout << "[INFO][Rank " << rank << "] " << message << std::endl;
}

void log_error(const std::string& message) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::cerr << "[ERROR][Rank " << rank << "] " << message << std::endl;
}
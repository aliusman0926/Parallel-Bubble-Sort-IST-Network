cmake_minimum_required(VERSION 3.10)
project(BubbleSortIST)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg") # Enable gprof profiling

find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)

include_directories(${MPI_INCLUDE_PATH} /usr/local/include)
link_directories(/usr/local/lib)

add_executable(bubble_sort_ist
    BubbleSortIST/src/main.cpp
    BubbleSortIST/src/graph/bubble_sort_graph.cpp
    BubbleSortIST/src/algorithm/ist_construct.cpp
    BubbleSortIST/src/parallel/metis_partition.cpp
    BubbleSortIST/src/parallel/mpi_utils.cpp
    BubbleSortIST/src/parallel/openmp_utils.cpp
    BubbleSortIST/src/utils/permutation.cpp
    BubbleSortIST/src/utils/logging.cpp
)

target_link_libraries(bubble_sort_ist
    ${MPI_LIBRARIES}
    OpenMP::OpenMP_CXX
    metis
)
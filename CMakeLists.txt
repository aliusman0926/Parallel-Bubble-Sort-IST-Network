cmake_minimum_required(VERSION 3.10)
project(BubbleSortIST LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)

# METIS (assumes installed in /usr/local)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${MPI_INCLUDE_PATH})

# Source files
set(SOURCES
    ${CMAKE_SOURCE_DIR}/BubbleSortIST/src/main.cpp
    ${CMAKE_SOURCE_DIR}/BubbleSortIST/src/graph/bubble_sort_graph.cpp
    ${CMAKE_SOURCE_DIR}/BubbleSortIST/src/algorithm/ist_construct.cpp
    ${CMAKE_SOURCE_DIR}/BubbleSortIST/src/parallel/metis_partition.cpp
    ${CMAKE_SOURCE_DIR}/BubbleSortIST/src/parallel/mpi_utils.cpp
    ${CMAKE_SOURCE_DIR}/BubbleSortIST/src/parallel/openmp_utils.cpp
    ${CMAKE_SOURCE_DIR}/BubbleSortIST/src/utils/permutation.cpp
    ${CMAKE_SOURCE_DIR}/BubbleSortIST/src/utils/logging.cpp
)

# Executable
add_executable(bubble_sort_ist ${SOURCES})
target_link_libraries(bubble_sort_ist PRIVATE MPI::MPI_CXX OpenMP::OpenMP_CXX metis)

# Enable testing (optional)
enable_testing()
find_package(GTest)
if(GTEST_FOUND)
    file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.cpp")
    add_executable(run_tests ${TEST_SOURCES} ${SOURCES})
    target_link_libraries(run_tests PRIVATE GTest::GTest GTest::Main MPI::MPI_CXX OpenMP::OpenMP_CXX metis)
    add_test(NAME UnitTests COMMAND run_tests)
else()
    message(STATUS "Google Test not found. Skipping unit tests.")
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")
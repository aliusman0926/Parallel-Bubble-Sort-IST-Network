#include "ist_construct.hpp"
#include "../utils/logging.hpp"
#include <unordered_map>
#include <stdexcept>

std::unordered_map<std::string, int> identity_cache;

int parent1(const BubbleSortGraph& graph, int v_idx, int t, int n) {
    if (v_idx < 0 || v_idx >= graph.num_vertices()) {
        log_error("Invalid vertex index: " + std::to_string(v_idx));
        throw std::runtime_error("Invalid vertex index");
    }
    if (t < 1 || t > n - 1) {
        log_error("Invalid tree index t: " + std::to_string(t));
        throw std::runtime_error("Invalid tree index");
    }

    log_info("Computing parent1 for vertex " + std::to_string(v_idx) + ", tree T_" + std::to_string(t));

    const Permutation& v = graph.vertices()[v_idx];
    const std::vector<int>& adj = graph.get_adjacent(v_idx, t - 1);
    if (adj.empty()) {
        log_info("No adjacent vertices for vertex " + std::to_string(v_idx) + " at t=" + std::to_string(t));
        return -1; // No parent
    }

    // Assume parent is the first adjacent vertex (simplified for bubble-sort graph)
    int parent_idx = adj[0];
    log_info("Parent of vertex " + std::to_string(v_idx) + " in T_" + std::to_string(t) + " is " + std::to_string(parent_idx));

    return parent_idx;
}
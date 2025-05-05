#include "permutation.hpp"
#include <sstream>

Permutation::Permutation(const std::vector<int>& p) : perm(p) {}

Permutation Permutation::swap(int i) const {
    std::vector<int> new_perm = perm;
    if (i >= 1 && i < static_cast<int>(perm.size())) {
        std::swap(new_perm[i - 1], new_perm[i]);
    }
    return Permutation(new_perm);
}

bool Permutation::operator==(const Permutation& other) const {
    return perm == other.perm;
}

bool Permutation::operator!=(const Permutation& other) const {
    return !(*this == other);
}

std::string Permutation::to_string() const {
    std::stringstream ss;
    for (int x : perm) {
        ss << x;
    }
    return ss.str();
}
#ifndef PERMUTATION_HPP
#define PERMUTATION_HPP

#include <vector>
#include <string>

class Permutation {
public:
    explicit Permutation(const std::vector<int>& p);
    Permutation swap(int i) const; // Swap positions i and i+1
    int operator[](int i) const { return perm[i]; }
    bool operator==(const Permutation& other) const;
    bool operator!=(const Permutation& other) const;
    std::string to_string() const;

private:
    std::vector<int> perm;
};

#endif
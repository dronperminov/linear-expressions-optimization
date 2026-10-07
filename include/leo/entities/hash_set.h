#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace leo {

class HashSet {
    std::vector<uint64_t> table;
    std::vector<uint64_t> states;
    size_t mask;
public:
    HashSet(size_t capacity);

    void insert(uint64_t key);
    bool contains(uint64_t key) const;
private:
    size_t key2index(uint64_t key) const;
    bool getState(size_t index) const;
};

} // namespace leo

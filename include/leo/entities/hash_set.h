#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace leo {

class HashSet {
    std::vector<uint64_t> table;
    uint64_t mask;
public:
    HashSet(size_t capacity);

    void insert(uint64_t key);
    bool contains(uint64_t key) const;
private:
    uint64_t normalize(uint64_t key) const;
    size_t key2index(uint64_t key) const;
};

} // namespace leo

#include <leo/entities/hash_set.h>

namespace leo {

HashSet::HashSet(size_t capacity) {
    size_t size = 16;

    while (size < 2 * capacity + 2)
        size <<= 1;

    table.assign(size, 0);
    mask = size - 1;
}

void HashSet::insert(uint64_t key) {
    key = normalize(key);

    size_t i = key2index(key);
    while (table[i] != 0) {
        if (table[i] == key)
            return;

        i = (i + 1) & mask;
    }

    table[i] = key;
}

bool HashSet::contains(uint64_t key) const {
    key = normalize(key);

    for (size_t i = key2index(key); table[i] != 0; i = (i + 1) & mask)
        if (table[i] == key)
            return true;

    return false;
}

uint64_t HashSet::normalize(uint64_t key) const {
    return key == 0 ? key - 1 : key;
}

size_t HashSet::key2index(uint64_t key) const {
    return ((key * 0x9e3779b97f4a7c15ULL) >> 20) & mask;
}

} // namespace leo

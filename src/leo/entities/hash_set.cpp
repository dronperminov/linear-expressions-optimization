#include <leo/entities/hash_set.h>

namespace leo {

HashSet::HashSet(size_t capacity) {
    size_t size = 16;

    while (size < 2 * capacity + 2)
        size <<= 1;

    table.assign(size, 0);
    states.assign(size, false);
    mask = size - 1;
}

void HashSet::insert(uint64_t key) {
    size_t i = key2index(key);

    while (states[i]) {
        if (table[i] == key)
            return;

        i = (i + 1) & mask;
    }

    table[i] = key;
    states[i] = true;
}

bool HashSet::contains(uint64_t key) const {
    for (size_t i = key2index(key); states[i]; i = (i + 1) & mask)
        if (table[i] == key)
            return true;

    return false;
}

size_t HashSet::key2index(uint64_t key) const {
    return ((key * 0x9e3779b97f4a7c15ULL) >> 20) & mask;
}

} // namespace leo

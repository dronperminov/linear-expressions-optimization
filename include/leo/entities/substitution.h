#pragma once

#include <cstddef>
#include <functional>

namespace leo {

struct Substitution {
    size_t i;
    size_t j;
    int ai;
    int aj;

    bool operator==(const Substitution& substitution) const;
    bool operator!=(const Substitution& substitution) const;
};

} // namespace leo

namespace std {
    template<>
    struct hash<leo::Substitution> {
        size_t operator()(const leo::Substitution& substitution) const;
    };
}

#include <leo/entities/substitution.h>

namespace leo {

bool Substitution::operator==(const Substitution& substitution) const {
    return i == substitution.i && j == substitution.j && ai == substitution.ai && aj == substitution.aj;
}

bool Substitution::operator!=(const Substitution& substitution) const {
    return !(*this == substitution);
}

} // namespace leo

namespace std {
size_t std::hash<leo::Substitution>::operator()(const leo::Substitution& substitution) const {
    size_t seed = 0;

    seed ^= std::hash<size_t>{}(substitution.i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<size_t>{}(substitution.j) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<int>{}(substitution.ai) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<int>{}(substitution.aj) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

    return seed;
}
}

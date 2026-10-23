#pragma once

#include <leo/entities/substitution.h>
#include <leo/entities/vector.h>

namespace leo::vec {

struct Candidate {
    Substitution step;
    Vector vector;
    Vector canonized;
    uint64_t fingerprint;
};

} // namespace leo::vec

#pragma once

#include <leo/entities/substitution.h>
#include <leo/entities/vector.h>

namespace leo::vector_covering {

struct Candidate {
    Substitution step;
    Vector vector;
    Vector canonized;
};

} // namespace leo::vector_covering

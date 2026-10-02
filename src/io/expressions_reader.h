#pragma once

#include <string>

#include <leo/entities/expressions_system.h>

class ExpressionsReader {
public:
    virtual leo::ExpressionsSystem read(const std::string& path) const = 0;

    virtual ~ExpressionsReader() = default;
};

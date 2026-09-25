#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <leo/entities/expressions_system.h>

class ExpressionsReader {
public:
    static leo::ExpressionsSystem read(const std::string& path);
};

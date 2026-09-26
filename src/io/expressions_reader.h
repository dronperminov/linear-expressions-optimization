#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <leo/entities/expressions_system.h>

class ExpressionsReader {
public:
    leo::ExpressionsSystem read(const std::string& path) const;
private:
    bool isBlankOrComment(const std::string& line) const;
    bool isInteger(const std::string& line, bool signedAvailable) const;

    std::pair<size_t, size_t> parseHeader(const std::string& line, const std::string& path, size_t lineNumber) const;
    std::vector<int> parseLine(const std::string& line, const std::string& path, size_t lineNumber) const;
    std::runtime_error makeError(const std::string& path, size_t lineNumber, const std::string& message) const;
};

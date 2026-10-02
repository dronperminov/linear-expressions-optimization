#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <leo/entities/expressions_system.h>

#include "../expressions_reader.h"
#include "../../utils.h"

class SmsExpressionsReader : public ExpressionsReader {
public:
    leo::ExpressionsSystem read(const std::string& path) const;
private:
    std::pair<size_t, size_t> parseHeader(const std::string& line, const std::string& path, size_t lineNumber) const;
    bool parseLine(const std::string& line, std::vector<std::vector<int>>& expressions, const std::string& path, size_t lineNumber) const;
    std::runtime_error makeError(const std::string& path, size_t lineNumber, const std::string& message) const;
};

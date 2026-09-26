#include "expressions_reader.h"

leo::ExpressionsSystem ExpressionsReader::read(const std::string& path) const {
    std::ifstream f(path);
    if (!f)
        throw std::runtime_error("failed to open \"" + path + "\"");

    std::string line;
    size_t lineNumber = 0;

    while (std::getline(f, line)) {
        lineNumber++;

        if (!isBlankOrComment(line))
            break;
    }

    if (f.eof() && isBlankOrComment(line))
        throw makeError(path, 0, "file is empty");

    auto [count, dimension] = parseHeader(line, path, lineNumber);

    std::vector<std::vector<int>> expressions;
    expressions.reserve(count);

    while (expressions.size() < count && std::getline(f, line)) {
        lineNumber++;

        if (isBlankOrComment(line))
            continue;

        std::vector<int> expression = parseLine(line, path, lineNumber);

        if (expression.size() != dimension) {
            std::ostringstream message;
            message << "expected " << dimension << " coefficients, got " << expression.size();
            throw makeError(path, lineNumber, message.str());
        }

        expressions.emplace_back(expression);
    }

    if (expressions.size() < count) {
        std::ostringstream message;
        message << "expected " << count << " expressions, got " << expressions.size();
        throw makeError(path, lineNumber, message.str());
    }

    while (std::getline(f, line)) {
        lineNumber++;

        if (!isBlankOrComment(line))
            throw makeError(path, lineNumber, "unexpected data after " + std::to_string(count) + " expressions");
    }

    return leo::ExpressionsSystem(expressions);
}

bool ExpressionsReader::isBlankOrComment(const std::string& line) const {
    return line.find_first_not_of(" \t\r\n") == std::string::npos || line.empty() || line[0] == '#';
}

bool ExpressionsReader::isInteger(const std::string& line, bool signedAvailable) const {
    if (line.empty())
        return false;

    size_t start = signedAvailable && line[0] == '-' ? 1 : 0;
    for (size_t i = start; i < line.size(); i++)
        if (line[i] < '0' || line[i] > '9')
            return false;

    return true;
}

std::pair<size_t, size_t> ExpressionsReader::parseHeader(const std::string& line, const std::string& path, size_t lineNumber) const {
    std::istringstream header(line);
    std::string countStr, dimensionStr;

    if (!(header >> countStr >> dimensionStr))
        throw makeError(path, lineNumber, "expected header \"<expressions> <dimension>\"");

    if (!isInteger(countStr, false))
        throw makeError(path, lineNumber, "number of expressions must be an integer (got " + countStr + ")");

    if (!isInteger(dimensionStr, false))
        throw makeError(path, lineNumber, "dimension must be an integer (got " + dimensionStr + ")");

    std::string extra;
    if (header >> extra)
        throw makeError(path, lineNumber, "header must contain exactly two integers: <expressions> <dimension>");

    size_t count = std::stoull(countStr);
    if (count == 0)
        throw makeError(path, lineNumber, "number of expressions must be greater than zero (got 0)");

    size_t dimension = std::stoull(dimensionStr);
    if (dimension == 0)
        throw makeError(path, lineNumber, "dimension must be greater than zero (got 0)");

    return {count, dimension};
}

std::vector<int> ExpressionsReader::parseLine(const std::string& line, const std::string& path, size_t lineNumber) const {
    std::istringstream ss(line);
    std::vector<int> values;
    std::string value;

    while (ss >> value) {
        if (!isInteger(value, true))
            throw makeError(path, lineNumber, "expected integer coefficients, got (" + value + ")");

        values.push_back(std::stoi(value));
    }

    if (ss.fail() && !ss.eof())
        throw makeError(path, lineNumber, "expected integer coefficients");

    return values;
}

std::runtime_error ExpressionsReader::makeError(const std::string& path, size_t lineNumber, const std::string& message) const {
    std::ostringstream ss;
    ss << "failed to read \"" << path << "\"";

    if (lineNumber > 0)
        ss << " at line " << lineNumber;

    ss << ": " << message;
    return std::runtime_error(ss.str());
}

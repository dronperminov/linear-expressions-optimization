#include "txt_expressions_reader.h"

leo::ExpressionsSystem TxtExpressionsReader::read(const std::string& path) const {
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
        throw makeError(path, 0, "file is empty: expected a header line \"<expressions> <dimension>\"");

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
            message << "wrong number of coefficients in expression #" << (expressions.size() + 1) << ": expected " << dimension << ", got " << expression.size();
            throw makeError(path, lineNumber, message.str());
        }

        expressions.emplace_back(expression);
    }

    if (expressions.size() < count) {
        std::ostringstream message;
        message << "unexpected end of file: expected " << count << " expressions, got " << expressions.size();
        throw makeError(path, lineNumber, message.str());
    }

    while (std::getline(f, line)) {
        lineNumber++;

        if (!isBlankOrComment(line))
            throw makeError(path, lineNumber, "unexpected data after " + std::to_string(count) + " expressions");
    }

    return leo::ExpressionsSystem(expressions);
}

std::pair<size_t, size_t> TxtExpressionsReader::parseHeader(const std::string& line, const std::string& path, size_t lineNumber) const {
    std::istringstream header(line);
    std::string countStr, dimensionStr;

    if (!(header >> countStr >> dimensionStr))
        throw makeError(path, lineNumber, "malformed header: expected \"<expressions> <dimension>\", got \"" + line + "\"");

    if (!isInteger(countStr, false))
        throw makeError(path, lineNumber, "invalid number of expressions: expected a non-negative integer, got \"" + countStr + "\"");

    if (!isInteger(dimensionStr, false))
        throw makeError(path, lineNumber, "invalid dimension: expected a non-negative integer, got \"" + dimensionStr + "\"");

    std::string extra;
    if (header >> extra)
        throw makeError(path, lineNumber, "malformed header: expected exactly \"<expressions> <dimension>\", found trailing token \"" + extra + "\"");

    size_t count = std::stoull(countStr);
    if (count == 0)
        throw makeError(path, lineNumber, "number of expressions must be positive, got 0");

    size_t dimension = std::stoull(dimensionStr);
    if (dimension == 0)
        throw makeError(path, lineNumber, "dimension must be positive, got 0");

    return {count, dimension};
}

std::vector<int> TxtExpressionsReader::parseLine(const std::string& line, const std::string& path, size_t lineNumber) const {
    std::istringstream ss(line);
    std::vector<int> values;
    std::string value;

    while (ss >> value) {
        if (!isInteger(value, true))
            throw makeError(path, lineNumber, "invalid coefficient: expected an integer, got \"" + value + "\"");

        values.push_back(std::stoi(value));
    }

    if (ss.fail() && !ss.eof())
        throw makeError(path, lineNumber, "malformed expression line: unexpected trailing data in \"" + line + "\"");

    return values;
}

std::runtime_error TxtExpressionsReader::makeError(const std::string& path, size_t lineNumber, const std::string& message) const {
    std::ostringstream ss;
    ss << "failed to read \"" << path << "\"";

    if (lineNumber > 0)
        ss << " at line " << lineNumber;

    ss << ": " << message;
    return std::runtime_error(ss.str());
}

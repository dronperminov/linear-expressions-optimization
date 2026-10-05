#include "sms_expressions_reader.h"

#include <fstream>
#include <sstream>

#include "../../utils.h"

leo::ExpressionsSystem SmsExpressionsReader::read(const std::string& path) const {
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
        throw makeError(path, 0, "file is empty: expected a header line \"<m> <n> R\"");

    auto [rows, columns] = parseHeader(line, path, lineNumber);

    std::vector<std::vector<int>> expressions(rows, std::vector<int>(columns, 0));

    while (std::getline(f, line)) {
        lineNumber++;

        if (isBlankOrComment(line))
            continue;

        if (parseLine(line, expressions, path, lineNumber))
            break;
    }

    while (std::getline(f, line)) {
        lineNumber++;

        if (!isBlankOrComment(line))
            throw makeError(path, lineNumber, "unexpected data after values");
    }

    return leo::ExpressionsSystem(expressions);
}

std::pair<size_t, size_t> SmsExpressionsReader::parseHeader(const std::string& line, const std::string& path, size_t lineNumber) const {
    std::istringstream header(line);
    std::string rowStr, columnsStr, letter;

    if (!(header >> rowStr >> columnsStr >> letter))
        throw makeError(path, lineNumber, "malformed header: expected header \"<m> <n> R\" (got \"" + line + "\")");

    if (!isInteger(rowStr, false))
        throw makeError(path, lineNumber, "invalid number of rows: expected a non-negative integer for <m>, got \"" + rowStr + "\"");

    if (!isInteger(columnsStr, false))
        throw makeError(path, lineNumber, "invalid number of columns: expected a non-negative integer for <n>, got \"" + columnsStr + "\"");

    if (letter != "R")
        throw makeError(path, lineNumber, "invalid field type: expected 'R' as the third header token, got \"" + letter + "\"");

    std::string extra;
    if (header >> extra)
        throw makeError(path, lineNumber, "malformed header: expected exactly \"<m> <n> 'R'\", found trailing token \"" + extra + "\"");

    size_t rows = std::stoull(rowStr);
    if (rows == 0)
        throw makeError(path, lineNumber, "number of rows (m) must be positive, got 0");

    size_t columns = std::stoull(columnsStr);
    if (columns == 0)
        throw makeError(path, lineNumber, "number of columns (m) must be positive, got 0");

    return {rows, columns};
}

bool SmsExpressionsReader::parseLine(const std::string& line, std::vector<std::vector<int>>& expressions, const std::string& path, size_t lineNumber) const {
    std::string rowStr, columnStr, valueStr;
    std::istringstream ss(line);

    if (!(ss >> rowStr >> columnStr >> valueStr))
        throw makeError(path, lineNumber, "malformed expression line: expected \"<row> <column> <value>\", got \"" + line + "\"");

    if (!isInteger(rowStr, false))
        throw makeError(path, lineNumber, "invalid row index: expected a non-negative integer, got \"" + rowStr + "\"");

    if (!isInteger(columnStr, false))
        throw makeError(path, lineNumber, "invalid column index: expected a non-negative integer, got \"" + columnStr + "\"");

    if (!isInteger(valueStr, true))
        throw makeError(path, lineNumber, "invalid value: expected an integer, got \"" + valueStr + "\"");

    if (ss.fail() && !ss.eof())
        throw makeError(path, lineNumber, "malformed expression line: expected exactly \"<row> <column> <value>\", found trailing data after \"" + valueStr + "\"");

    size_t row = std::stoull(rowStr);
    size_t column = std::stoull(columnStr);
    int value = std::stoi(valueStr);

    if (row == 0 && column == 0 && value == 0)
        return true;

    if (row < 1 || row > expressions.size())
        throw makeError(path, lineNumber, "row index " + rowStr + " is out of range: valid rows are 1.." + std::to_string(expressions.size()));

    if (column < 1 || column > expressions[0].size())
        throw makeError(path, lineNumber, "column index " + columnStr + " is out of range: valid columns are 1.." + std::to_string(expressions[0].size()));

    expressions[row - 1][column - 1] = value;
    return false;
}

std::runtime_error SmsExpressionsReader::makeError(const std::string& path, size_t lineNumber, const std::string& message) const {
    std::ostringstream ss;
    ss << "failed to read SMS data from \"" << path << "\"";

    if (lineNumber > 0)
        ss << " at line " << lineNumber;

    ss << ": " << message;
    return std::runtime_error(ss.str());
}

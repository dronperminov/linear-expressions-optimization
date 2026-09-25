#include "expressions_reader.h"

leo::ExpressionsSystem ExpressionsReader::read(const std::string& path) {
    std::ifstream f(path);
    if (!f)
        throw std::runtime_error("ExpressionsReader::read: unable to open file \"" + path + "\"");

    int count;
    int dimension;

    f >> count >> dimension;
    if (count <= 0)
        throw std::runtime_error("ExpressionsReader::read: invalid number of expressions (" + std::to_string(count) + ")");

    if (dimension <= 0)
        throw std::runtime_error("ExpressionsReader::read: invalid expressions dimension (" + std::to_string(dimension) + ")");

    std::vector<std::vector<int>> expressions(count, std::vector<int>(dimension, 0));

    // TODO: errors handling
    for (int i = 0; i < count; i++)
        for (int j = 0; j < dimension; j++)
            f >> expressions[i][j];

    f.close();
    return leo::ExpressionsSystem(expressions);
}

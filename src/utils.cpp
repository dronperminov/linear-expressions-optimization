#include "utils.h"

bool isBlankOrComment(const std::string& line) {
    size_t index = line.find_first_not_of(" \t\r\n");
    return index == std::string::npos || line.empty() || line[index] == '#';
}

bool isInteger(const std::string& line, bool signedAvailable) {
    if (line.empty())
        return false;

    size_t start = signedAvailable && line[0] == '-' ? 1 : 0;
    for (size_t i = start; i < line.size(); i++)
        if (line[i] < '0' || line[i] > '9')
            return false;

    return true;
}

std::string formatDuration(std::chrono::steady_clock::duration duration) {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::ostringstream oss;

    if (ms < 1000)
        return std::to_string(ms) + " ms";

    double elapsed = ms / 1000.0;

    if (elapsed < 60) {
        oss << std::setprecision(2) << std::fixed << elapsed << " sec";
    }
    else {
        int seconds = int(elapsed + 0.5);
        int hours = seconds / 3600;
        int minutes = (seconds % 3600) / 60;

        oss << std::setw(2) << std::setfill('0') << hours << ":";
        oss << std::setw(2) << std::setfill('0') << minutes << ":";
        oss << std::setw(2) << std::setfill('0') << (seconds % 60);
    }

    return oss.str();
}

std::string replace(const std::string& str, const std::string& from, const std::string& to) {
    if (from.empty())
        return str;

    std::string result = str;
    size_t start = result.find(from);

    while (start != std::string::npos) {
        result.replace(start, from.length(), to);
        start = result.find(from, start + to.length());
    }

    return result;
}

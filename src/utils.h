#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

bool isBlankOrComment(const std::string& line);
bool isInteger(const std::string& line, bool signedAvailable);

std::string formatDuration(std::chrono::steady_clock::duration duration);
std::string replace(const std::string& str, const std::string& from, const std::string& to);

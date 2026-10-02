#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

bool isBlankOrComment(const std::string& line);
bool isInteger(const std::string& line, bool signedAvailable);

std::string formatDuration(std::chrono::steady_clock::duration duration);

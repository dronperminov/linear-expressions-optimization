#pragma once

#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "argument.h"

class ArgParser {
    std::string name;
    std::string description;
    std::vector<Argument> arguments;
    std::vector<std::string> sections;
    std::unordered_map<std::string, size_t> arg2index;
    size_t maxArgWidth;
public:
    ArgParser(const std::string &name, const std::string &description = "");

    void addSection(const std::string &sectionName);
    void addChoices(const std::string &longName, const std::string &shortName, ArgType type, const std::string &description, const std::vector<std::string> &choices, const std::string &defaultValue, bool required = false);
    void addChoices(const std::string &name, ArgType type, const std::string &description, const std::vector<std::string> &choices, const std::string &defaultValue, bool required = false);

    void add(const std::string &longName, const std::string &shortName, ArgType type, const std::string &description, const std::string &defaultValue = "", bool required = false);
    void add(const std::string &name, ArgType type, const std::string &description, const std::string &defaultValue = "", bool required = false);

    bool parse(int argc, char** argv);
    bool isSet(const std::string &name) const;

    void help() const;

    std::string get(const std::string &name) const;
    std::string operator[](const std::string &name) const;
};

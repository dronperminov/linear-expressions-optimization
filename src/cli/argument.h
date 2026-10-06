#pragma once

#include <cstddef>
#include <string>
#include <vector>

enum class ArgType {
    String, Path, Natural, UInt, Real, Flag
};

struct Argument {
    std::string longName;
    std::string shortName;
    ArgType type;
    std::string description;
    std::string section;

    std::string defaultValue;
    std::vector<std::string> choices;
    std::string value;
    bool required;
    bool isSet;

    Argument(const std::string& longName, const std::string& shortName, ArgType type, const std::string& description, const std::string& section, const std::vector<std::string>& choices, const std::string& defaultValue, bool required);

    std::string getHelpName() const;
    std::string getHelpDescription() const;
    std::string getTypeHint() const;
    size_t getWidth() const;

    bool validate(const std::string& parsedName) const;
private:
    bool isNatural(const std::string& value) const;
    bool isReal(const std::string& value) const;
    bool isValidChoice(const std::string& value) const;
};

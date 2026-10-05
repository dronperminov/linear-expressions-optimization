#include "argument.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

Argument::Argument(const std::string& longName, const std::string& shortName, ArgType type, const std::string& description, const std::string& section, const std::vector<std::string>& choices, const std::string& defaultValue, bool required) {
    this->longName = longName;
    this->shortName = shortName;
    this->type = type;
    this->description = description;
    this->section = section;
    this->choices = choices;
    this->defaultValue = defaultValue;
    this->value = defaultValue;
    this->required = required;
    this->isSet = false;
}

std::string Argument::getHelpName() const {
    std::stringstream ss;

    if (longName.empty()) {
        ss << shortName;
    }
    else if (shortName.empty()) {
        ss << longName;
    }
    else {
        ss << shortName + ", " + longName;
    }

    ss << " " << getTypeHint();
    return ss.str();
}

std::string Argument::getHelpDescription() const {
    std::stringstream ss;
    ss << description;

    if (type != ArgType::Flag && !defaultValue.empty())
        ss << " (default: " << defaultValue << ")";

    if (required)
        ss << " [REQUIRED]";

    return ss.str();
}

std::string Argument::getTypeHint() const {
    if (!choices.empty()) {
        std::stringstream ss;
        ss << "{";

        for (size_t i = 0; i < choices.size(); i++)
            ss << (i > 0 ? ", " : "") << choices[i];

        ss << "}";
        return ss.str();
    }

    if (type == ArgType::Natural)
        return "<natural>";

    if (type == ArgType::UInt)
        return "<int>";

    if (type == ArgType::Real)
        return "<real>";

    if (type == ArgType::String)
        return "<str>";

    if (type == ArgType::Path)
        return "<path>";

    return "";
}

size_t Argument::getWidth() const {
    size_t typeHintWidth = 1 + getTypeHint().length();

    if (longName.empty())
        return shortName.length() + typeHintWidth;

    if (shortName.empty())
        return longName.length() + typeHintWidth;

    return longName.length() + 2 + shortName.length() + typeHintWidth;
}

bool Argument::validate(const std::string& parsedName) const {
    if (type != ArgType::Flag && !isValidChoice(value)) {
        std::cerr << "Invalid value for argument \"" << parsedName << "\": " << value << " is not valid choice. Valid choices are: ";

        for (size_t i = 0; i < choices.size(); i++)
            std::cerr << (i > 0 ? ", " : "") << choices[i];

        std::cerr << std::endl;
        return false;
    }

    if (type == ArgType::String || type == ArgType::Path)
        return true;

    if (type == ArgType::Natural && !isNatural(value)) {
        std::cerr << "Invalid value for argument \"" << parsedName << "\": " << value << " is not natural" << std::endl;
        return false;
    }

    if (type == ArgType::UInt && !isNatural(value) && value != "0") {
        std::cerr << "Invalid value for argument \"" << parsedName << "\": " << value << " is not unsigned integer" << std::endl;
        return false;
    }

    if (type == ArgType::Real && !isReal(value)) {
        std::cerr << "Invalid value for argument \"" << parsedName << "\": " << value << " is not real" << std::endl;
        return false;
    }

    return true;
}

bool Argument::isNatural(const std::string& value) const {
    for (size_t i = 0; i < value.size(); i++)
        if (value[i] < '0' || value[i] > '9')
            return false;

    return std::stoull(value) > 0;
}

bool Argument::isReal(const std::string& value) const {
    if (value.size() == 0)
        return false;

    size_t start = value[0] == '-' ? 1 : 0;
    bool point = false;

    for (size_t i = start; i < value.size(); i++) {
        if (value[i] == '.') {
            if (point)
                return false;

            point = true;
        }
        else if (value[i] < '0' || value[i] > '9')
            return false;
    }

    return true;
}

bool Argument::isValidChoice(const std::string& value) const {
    if (choices.empty())
        return true;

    return std::find(choices.begin(), choices.end(), value) != choices.end();
}

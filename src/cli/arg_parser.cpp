#include "arg_parser.h"

#include <iomanip>
#include <iostream>

ArgParser::ArgParser(const std::string& name, const std::string& description) {
    this->name = name;
    this->description = description;
    this->maxArgWidth = 30;

    addSection("Main options");
    add("--help", "-h", ArgType::Flag, "Show this help message");
}

void ArgParser::addSection(const std::string& section) {
    sections.push_back(section);
}

void ArgParser::addChoices(const std::string& longName, const std::string& shortName, ArgType type, const std::string& description, const std::vector<std::string>& choices, const std::string& defaultValue, bool required) {
    Argument argument(longName, shortName, type, description, sections.back(), choices, defaultValue, required);
    arguments.emplace_back(argument);

    if (!longName.empty())
        arg2index[longName] = arguments.size() - 1;

    if (!shortName.empty())
        arg2index[shortName] = arguments.size() - 1;

    maxArgWidth = std::max(maxArgWidth, argument.getWidth());
}

void ArgParser::addChoices(const std::string& name, ArgType type, const std::string& description, const std::vector<std::string>& choices, const std::string& defaultValue, bool required) {
    if (name.find("--") == 0) {
        addChoices(name, "", type, description, choices, defaultValue, required);
    }
    else if (name.find("-") == 0) {
        addChoices("", name, type, description, choices, defaultValue, required);
    }
    else
        throw std::runtime_error("Argument name must starts with \"-\" or \"--\"");
}

void ArgParser::add(const std::string& longName, const std::string& shortName, ArgType type, const std::string& description, const std::string& defaultValue, bool required) {
    addChoices(longName, shortName, type, description, {}, defaultValue, required);
}

void ArgParser::add(const std::string& name, ArgType type, const std::string& description, const std::string& defaultValue, bool required) {
    addChoices(name, type, description, {}, defaultValue, required);
}

bool ArgParser::parse(int argc, char** argv) {
    if (argc > 0)
        name = argv[0];

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            help();
            return false;
        }

        auto it = arg2index.find(arg);
        if (it == arg2index.end()) {
            std::cerr << "Unknown argument \"" << arg << "\"" << std::endl;
            return false;
        }

        Argument& argument = arguments[it->second];
        if (argument.isSet) {
            std::cerr << "Argument \"" << arg << "\" has already been set" << std::endl;
            return false;
        }

        argument.isSet = true;

        if (argument.type == ArgType::Flag) {
            argument.value = "true";
            continue;
        }

        if (i + 1 >= argc) {
            std::cerr << "Missing value for argument \"" << arg << "\"" << std::endl;
            return false;
        }

        argument.value = argv[++i];
        if (!argument.validate(arg))
            return false;
    }

    for (const auto& argument : arguments) {
        if (argument.required && !argument.isSet) {
            std::cerr << "Required argument \"" << (argument.longName.empty() ? argument.shortName : argument.longName) << "\" is missing" << std::endl;
            return false;
        }
    }

    return true;
}

bool ArgParser::isSet(const std::string& name) const {
    auto it = arg2index.find(name);
    if (it == arg2index.end())
        return false;

    return arguments[it->second].isSet;
}

void ArgParser::help() const {
    std::cout << description << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: " << name << " [ARGS...]" << std::endl;

    std::unordered_map<std::string, std::vector<Argument>> section2args;

    for (const Argument& argument : arguments)
        section2args[argument.section].push_back(argument);

    for (const std::string& section : sections) {
        if (section2args.at(section).empty())
            continue;

        std::cout << std::endl << section << ":" << std::endl;

        for (const Argument& argument : section2args.at(section)) {
            std::cout << "  " << std::left << std::setw(maxArgWidth + 2) << argument.getHelpName();
            std::cout << argument.getHelpDescription();
            std::cout << std::endl;
        }
    }
}

std::string ArgParser::get(const std::string& name) const {
    auto it = arg2index.find(name);
    if (it == arg2index.end())
        throw std::runtime_error("Argument not found: " + name);

    return arguments[it->second].value;
}

std::string ArgParser::operator[](const std::string& name) const {
    return get(name);
}

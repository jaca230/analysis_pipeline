#ifndef ANALYSISPIPELINE_CONFIGPARSER_H
#define ANALYSISPIPELINE_CONFIGPARSER_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class ConfigParser {
public:
    ConfigParser() = default;

    // Reads a single JSON file into a nlohmann::json object
    // Returns true if successful, false otherwise
    bool parseFile(const std::string& filepath, nlohmann::json& outJson) const;
};

#endif // ANALYSISPIPELINE_CONFIGPARSER_H

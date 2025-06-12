#include "config_parser.h"
#include <fstream>
#include <iostream>

bool ConfigParser::parseFile(const std::string& filepath, nlohmann::json& outJson) const {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        std::cerr << "[ConfigParser] Could not open config file: " << filepath << std::endl;
        return false;
    }

    try {
        ifs >> outJson;
    } catch (const std::exception& e) {
        std::cerr << "[ConfigParser] Failed to parse JSON: " << e.what() << std::endl;
        return false;
    }

    return true;
}

#include "config_parser.h"
#include <fstream>
#include <iostream>

bool ConfigParser::parseFile(const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        std::cerr << "[ConfigParser] ERROR: Could not open config file: " << filepath << std::endl;
        return false;
    }

    nlohmann::json j;
    try {
        ifs >> j;
    } catch (const std::exception& e) {
        std::cerr << "[ConfigParser] ERROR: Failed to parse JSON: " << e.what() << std::endl;
        return false;
    }

    if (!j.contains("pipeline") || !j["pipeline"].is_array()) {
        std::cerr << "[ConfigParser] ERROR: Config JSON does not contain a valid 'pipeline' array." << std::endl;
        return false;
    }

    stages_.clear();
    for (const auto& stage : j["pipeline"]) {
        StageConfig sc;
        if (!stage.contains("id") || !stage.contains("type") || !stage.contains("parameters") || !stage.contains("next")) {
            std::cerr << "[ConfigParser] ERROR: A stage is missing required fields." << std::endl;
            return false;
        }

        sc.id = stage["id"].get<std::string>();
        sc.type = stage["type"].get<std::string>();
        sc.parameters = stage["parameters"];
        sc.next = stage["next"].get<std::vector<std::string>>();
        stages_.push_back(std::move(sc));
    }

    return true;
}

const std::vector<StageConfig>& ConfigParser::getStages() const {
    return stages_;
}

#include "config_manager.h"
#include <iostream>

ConfigManager::ConfigManager() : mergedJson_(nlohmann::json::object()) {}

bool ConfigManager::loadFiles(const std::vector<std::string>& filepaths) {
    mergedJson_ = nlohmann::json::object();

    for (const auto& filepath : filepaths) {
        nlohmann::json j;
        if (!parser_.parseFile(filepath, j)) {
            std::cerr << "[ConfigManager] Failed to parse config file: " << filepath << std::endl;
            return false;
        }
        if (!mergeJson(j)) {
            std::cerr << "[ConfigManager] Failed to merge config from file: " << filepath << std::endl;
            return false;
        }
    }

    // Automatically build internal structs after merge
    if (!buildFromMergedConfig()) {
        std::cerr << "[ConfigManager] Failed to build configuration from merged JSON." << std::endl;
        return false;
    }

    return true;
}


bool ConfigManager::mergeJson(const nlohmann::json& newJson) {
    // Simple shallow merge: keys from newJson overwrite or add to mergedJson_
    for (auto it = newJson.begin(); it != newJson.end(); ++it) {
        mergedJson_[it.key()] = it.value();
    }
    return true;
}

bool ConfigManager::buildFromMergedConfig() {
    pipelineStages_.clear();
    loggerConfig_.raw.clear();

    // Parse pipeline stages
    if (mergedJson_.contains("pipeline")) {
        if (!parsePipelineStages(mergedJson_["pipeline"])) {
            std::cerr << "[ConfigManager] Failed to parse pipeline stages." << std::endl;
            return false;
        }
    } else {
        std::cerr << "[ConfigManager] Missing 'pipeline' key in merged config." << std::endl;
        return false;
    }

    // Parse logger config (optional)
    if (mergedJson_.contains("logger")) {
        if (!parseLoggerConfig(mergedJson_["logger"])) {
            std::cerr << "[ConfigManager] Failed to parse logger config." << std::endl;
            return false;
        }
    } // else no logger config is okay

    return true;
}

bool ConfigManager::parsePipelineStages(const nlohmann::json& pipelineJson) {
    if (!pipelineJson.is_array()) {
        std::cerr << "[ConfigManager] 'pipeline' is not an array." << std::endl;
        return false;
    }

    for (const auto& stage : pipelineJson) {
        if (!stage.contains("id") || !stage.contains("type") || !stage.contains("parameters") || !stage.contains("next")) {
            std::cerr << "[ConfigManager] A pipeline stage is missing required fields." << std::endl;
            return false;
        }
        StageConfig sc;
        try {
            sc.id = stage.at("id").get<std::string>();
            sc.type = stage.at("type").get<std::string>();
            sc.parameters = stage.at("parameters");
            sc.next = stage.at("next").get<std::vector<std::string>>();
        } catch (const std::exception& e) {
            std::cerr << "[ConfigManager] Exception parsing stage: " << e.what() << std::endl;
            return false;
        }
        pipelineStages_.push_back(std::move(sc));
    }
    return true;
}

bool ConfigManager::parseLoggerConfig(const nlohmann::json& loggerJson) {
    // Here just store the raw logger json
    loggerConfig_.raw = loggerJson;
    return true;
}

bool ConfigManager::validate() const {
    if (pipelineStages_.empty()) {
        std::cerr << "[ConfigManager] Validation failed: pipeline stages empty." << std::endl;
        return false;
    }

    std::set<std::string> ids;
    for (const auto& stage : pipelineStages_) {
        if (stage.id.empty()) {
            std::cerr << "[ConfigManager] Validation failed: stage with empty id." << std::endl;
            return false;
        }
        if (!ids.insert(stage.id).second) {
            std::cerr << "[ConfigManager] Validation failed: duplicate stage id '" << stage.id << "'." << std::endl;
            return false;
        }
    }

    return true;
}

const std::vector<StageConfig>& ConfigManager::getPipelineStages() const {
    return pipelineStages_;
}

const LoggerConfig& ConfigManager::getLoggerConfig() const {
    return loggerConfig_;
}

void ConfigManager::setPipelineStages(const std::vector<StageConfig>& stages) {
    pipelineStages_ = stages;
}

void ConfigManager::setLoggerConfig(const LoggerConfig& loggerConfig) {
    loggerConfig_ = loggerConfig;
}

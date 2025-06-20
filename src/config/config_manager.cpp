#include "config/config_manager.h"
#include <iostream>

ConfigManager::ConfigManager() : mergedJson_(nlohmann::json::object()) {}

void ConfigManager::reset() {
    mergedJson_ = nlohmann::json::object();
    pipelineStages_.clear();
    loggerConfig_.raw.clear();
}

bool ConfigManager::loadFiles(const std::vector<std::string>& filepaths) {
    reset();

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

    return buildFromMergedConfig();
}

bool ConfigManager::addJsonObject(const nlohmann::json& j) {
    if (!mergeJson(j)) {
        std::cerr << "[ConfigManager] Failed to merge JSON object." << std::endl;
        return false;
    }

    return buildFromMergedConfig();
}

bool ConfigManager::mergeJson(const nlohmann::json& newJson) {
    for (auto it = newJson.begin(); it != newJson.end(); ++it) {
        mergedJson_[it.key()] = it.value();
    }
    return true;
}

bool ConfigManager::buildFromMergedConfig() {
    pipelineStages_.clear();
    loggerConfig_.raw.clear();

    if (!mergedJson_.contains("pipeline")) {
        std::cerr << "[ConfigManager] Missing 'pipeline' key in config." << std::endl;
        return false;
    }

    if (!parsePipelineStages(mergedJson_["pipeline"])) {
        std::cerr << "[ConfigManager] Failed to parse pipeline stages." << std::endl;
        return false;
    }

    if (mergedJson_.contains("logger")) {
        if (!parseLoggerConfig(mergedJson_["logger"])) {
            std::cerr << "[ConfigManager] Failed to parse logger config." << std::endl;
            return false;
        }
    }

    return true;
}

bool ConfigManager::parsePipelineStages(const nlohmann::json& pipelineJson) {
    if (!pipelineJson.is_array()) {
        std::cerr << "[ConfigManager] 'pipeline' must be an array." << std::endl;
        return false;
    }

    for (const auto& stage : pipelineJson) {
        if (!stage.contains("id") || !stage.contains("type") ||
            !stage.contains("parameters") || !stage.contains("next")) {
            std::cerr << "[ConfigManager] A stage is missing required fields." << std::endl;
            return false;
        }

        try {
            StageConfig sc;
            sc.id = stage.at("id").get<std::string>();
            sc.type = stage.at("type").get<std::string>();
            sc.parameters = stage.at("parameters");
            sc.next = stage.at("next").get<std::vector<std::string>>();
            pipelineStages_.push_back(std::move(sc));
        } catch (const std::exception& e) {
            std::cerr << "[ConfigManager] Exception parsing stage: " << e.what() << std::endl;
            return false;
        }
    }

    return true;
}

bool ConfigManager::parseLoggerConfig(const nlohmann::json& loggerJson) {
    loggerConfig_.raw = loggerJson;
    return true;
}

bool ConfigManager::validate() const {
    if (pipelineStages_.empty()) {
        std::cerr << "[ConfigManager] Validation failed: no pipeline stages." << std::endl;
        return false;
    }

    std::set<std::string> ids;
    for (const auto& stage : pipelineStages_) {
        if (stage.id.empty()) {
            std::cerr << "[ConfigManager] Validation failed: empty stage id." << std::endl;
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

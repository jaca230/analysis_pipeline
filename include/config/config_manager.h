#ifndef ANALYSISPIPELINE_CONFIGMANAGER_H
#define ANALYSISPIPELINE_CONFIGMANAGER_H

#include <string>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>
#include "config/config_parser.h"

struct StageConfig {
    std::string id;
    std::string type;
    nlohmann::json parameters;
    std::vector<std::string> next;
};

struct LoggerConfig {
    nlohmann::json raw;
};

class ConfigManager {
public:
    ConfigManager();

    // Load and merge JSON config files
    bool loadFiles(const std::vector<std::string>& filepaths);

    // Add a JSON object directly
    bool addJsonObject(const nlohmann::json& j);

    // Reset internal config state
    void reset();

    // Validate internal pipeline config
    bool validate() const;

    const std::vector<StageConfig>& getPipelineStages() const;
    const LoggerConfig& getLoggerConfig() const;

    void setPipelineStages(const std::vector<StageConfig>& stages);
    void setLoggerConfig(const LoggerConfig& loggerConfig);

private:
    ConfigParser parser_;
    nlohmann::json mergedJson_;
    std::vector<StageConfig> pipelineStages_;
    LoggerConfig loggerConfig_;

    bool mergeJson(const nlohmann::json& newJson);
    bool buildFromMergedConfig();
    bool parsePipelineStages(const nlohmann::json& j);
    bool parseLoggerConfig(const nlohmann::json& j);
};

#endif // ANALYSISPIPELINE_CONFIGMANAGER_H

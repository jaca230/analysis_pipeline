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

class ConfigManager {
public:
    ConfigManager();

    // Load and merge JSON config files
    bool loadFiles(const std::vector<std::string>& filepaths);
    bool addJsonObject(const nlohmann::json& j);
    void reset();
    bool validate() const;

    const std::vector<StageConfig>& getPipelineStages() const;
    const nlohmann::json& getLoggerConfig() const;
    const std::vector<std::string>& getPluginLibraries() const;

    void setPipelineStages(const std::vector<StageConfig>& stages);
    void setLoggerConfig(const nlohmann::json& loggerJson);
    void setPluginLibraries(const std::vector<std::string>& libs);

private:
    ConfigParser parser_;
    nlohmann::json mergedJson_;
    std::vector<StageConfig> pipelineStages_;
    nlohmann::json loggerConfig_;
    std::vector<std::string> pluginLibraries_;

    bool mergeJson(const nlohmann::json& newJson);
    bool buildFromMergedConfig();
    bool parsePipelineStages(const nlohmann::json& j);
};

#endif // ANALYSISPIPELINE_CONFIGMANAGER_H

#ifndef ANALYSISPIPELINE_CONFIGPARSER_H
#define ANALYSISPIPELINE_CONFIGPARSER_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct StageConfig {
    std::string id;
    std::string type;
    nlohmann::json parameters;
    std::vector<std::string> next;
};

class ConfigParser {
public:
    ConfigParser() = default;
    bool parseFile(const std::string& filepath);
    const std::vector<StageConfig>& getStages() const;

private:
    std::vector<StageConfig> stages_;
};

#endif // ANALYSISPIPELINE_CONFIGPARSER_H

#ifndef ANALYSISPIPELINE_PIPELINE_H
#define ANALYSISPIPELINE_PIPELINE_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <optional>
#include <tbb/flow_graph.h>
#include <TObject.h>
#include "config_manager.h"  // Use ConfigManager instead of raw configs
#include "base_stage.h"

class Pipeline {
public:
    // Pipeline requires a ConfigManager reference to be constructed
    explicit Pipeline(std::shared_ptr<ConfigManager> configManager);

    // Build graph from the current config manager's loaded config
    bool buildFromConfig();

    void execute();

    // Getters and setters for config manager
    std::shared_ptr<ConfigManager> getConfigManager() const;
    void setConfigManager(std::shared_ptr<ConfigManager> configManager);

private:
    tbb::flow::graph graph_;
    std::map<std::string, std::unique_ptr<tbb::flow::continue_node<tbb::flow::continue_msg>>> nodes_;
    std::map<std::string, int> incomingCount_;
    std::vector<std::string> startNodes_;

    std::shared_ptr<ConfigManager> configManager_;

    BaseStage* createStageInstance(const std::string& type, const nlohmann::json& params);
    void configureLogger(const nlohmann::json& loggerConfig);
};

#endif // ANALYSISPIPELINE_PIPELINE_H

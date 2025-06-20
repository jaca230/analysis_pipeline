#ifndef ANALYSISPIPELINE_PIPELINE_H
#define ANALYSISPIPELINE_PIPELINE_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <optional>
#include <tbb/flow_graph.h>
#include "config/config_manager.h"
#include "stages/base_stage.h"
#include "data/pipeline_data_product_manager.h" 

class Pipeline {
public:
    explicit Pipeline(std::shared_ptr<ConfigManager> configManager);

    bool buildFromConfig();

    void execute();

    std::shared_ptr<ConfigManager> getConfigManager() const;
    void setConfigManager(std::shared_ptr<ConfigManager> configManager);

    // Accessor for the data product manager
    PipelineDataProductManager& getDataProductManager();

private:
    tbb::flow::graph graph_;
    std::map<std::string, std::unique_ptr<tbb::flow::continue_node<tbb::flow::continue_msg>>> nodes_;
    std::map<std::string, int> incomingCount_;
    std::vector<std::string> startNodes_;

    std::shared_ptr<ConfigManager> configManager_;

    // Use PipelineDataProductManager instead of raw map+mutex
    PipelineDataProductManager dataProductManager_;

    BaseStage* createStageInstance(const std::string& type, const nlohmann::json& params);
    void configureLogger(const nlohmann::json& loggerConfig);
};

#endif // ANALYSISPIPELINE_PIPELINE_H

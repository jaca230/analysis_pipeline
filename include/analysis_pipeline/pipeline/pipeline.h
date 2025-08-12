#ifndef ANALYSISPIPELINE_PIPELINE_H
#define ANALYSISPIPELINE_PIPELINE_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <optional>
#include <any>
#include <tbb/flow_graph.h>

#include "analysis_pipeline/config/config_manager.h"
#include "analysis_pipeline/core/stages/base_stage.h"
#include "analysis_pipeline/core/stages/input/base_input_stage.h"
#include "analysis_pipeline/core/data/pipeline_data_product_manager.h"
#include "analysis_pipeline/core/context/input_bundle.h"

class Pipeline {
public:
    explicit Pipeline(std::shared_ptr<ConfigManager> configManager);

    bool buildFromConfig();

    void execute();

    std::shared_ptr<ConfigManager> getConfigManager() const;
    void setConfigManager(std::shared_ptr<ConfigManager> configManager);

    // Accessor for the data product manager
    PipelineDataProductManager& getDataProductManager();

    // Generic input injection method
    void setInputData(const InputBundle& input);

    // Setter for conditional ROOT thread safety enabling
    void setEnableThreadSafetyIfNeeded(bool enable);

private:
    tbb::flow::graph graph_;
    std::map<std::string, std::unique_ptr<tbb::flow::continue_node<tbb::flow::continue_msg>>> nodes_;
    std::map<std::string, int> incomingCount_;
    std::vector<std::string> startNodes_;
    std::map<std::string, std::unique_ptr<BaseStage>> stages_;

    // Collection of input stages (BaseInputStage*)
    std::vector<BaseInputStage*> input_stages_;

    std::shared_ptr<ConfigManager> configManager_;

    PipelineDataProductManager dataProductManager_;

    // Controls whether to enable ROOT::EnableThreadSafety() based on parallelism detection
    bool enable_thread_safety_if_needed_ = true;

    BaseStage* createStageInstance(const std::string& type, const nlohmann::json& params);
    void configureLogger(const nlohmann::json& loggerConfig);

    void registerInputStage(BaseInputStage* stage);

    // Internal helper to enable ROOT thread safety if conditions are met
    void enableRootThreadSafetyIfNeeded();
};

#endif // ANALYSISPIPELINE_PIPELINE_H

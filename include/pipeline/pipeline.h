#ifndef ANALYSISPIPELINE_PIPELINE_H
#define ANALYSISPIPELINE_PIPELINE_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <optional>
#include <mutex>
#include <tbb/flow_graph.h>
#include <TObject.h>
#include <TTree.h>
#include "config_manager.h"  // Use ConfigManager instead of raw configs
#include "base_stage.h"

class Pipeline {
public:
    explicit Pipeline(std::shared_ptr<ConfigManager> configManager);

    bool buildFromConfig();

    void execute();

    std::shared_ptr<ConfigManager> getConfigManager() const;
    void setConfigManager(std::shared_ptr<ConfigManager> configManager);

    // Thread-safe accessors for the owned TTree
    TTree* getTree() const;
    void setTree(TTree* tree);

private:
    tbb::flow::graph graph_;
    std::map<std::string, std::unique_ptr<tbb::flow::continue_node<tbb::flow::continue_msg>>> nodes_;
    std::map<std::string, int> incomingCount_;
    std::vector<std::string> startNodes_;

    std::shared_ptr<ConfigManager> configManager_;

    // Own the TTree and mutex for thread safety
    TTree* tree_ = nullptr;
    mutable std::mutex tree_mutex_;

    BaseStage* createStageInstance(const std::string& type, const nlohmann::json& params);
    void configureLogger(const nlohmann::json& loggerConfig);
};

#endif // ANALYSISPIPELINE_PIPELINE_H

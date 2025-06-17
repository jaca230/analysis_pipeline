#ifndef ANALYSISPIPELINE_PIPELINE_H
#define ANALYSISPIPELINE_PIPELINE_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <mutex>
#include <optional>

#include <tbb/flow_graph.h>
#include <TObject.h>
#include <TTree.h>

#include "config/config_manager.h"
#include "stages/base_stage.h"
#include "stages/base_midas_unpacker_stage.h"
#include "midas.h"  // For TMEvent

class Pipeline {
public:
    explicit Pipeline(std::shared_ptr<ConfigManager> configManager);

    bool buildFromConfig();
    void execute();

    std::shared_ptr<ConfigManager> getConfigManager() const;
    void setConfigManager(std::shared_ptr<ConfigManager> configManager);

    TTree* getTree() const;
    void setTree(TTree* tree);

    void setCurrentEvent(const TMEvent& event);

private:
    tbb::flow::graph graph_;
    std::map<std::string, std::unique_ptr<tbb::flow::continue_node<tbb::flow::continue_msg>>> nodes_;
    std::map<std::string, std::unique_ptr<BaseStage>> stages_;
    std::map<std::string, int> incomingCount_;
    std::vector<std::string> startNodes_;

    std::shared_ptr<ConfigManager> configManager_;
    TTree* tree_ = nullptr;
    mutable std::mutex tree_mutex_;

    std::vector<BaseMidasUnpackerStage*> midas_unpackers_;

    BaseStage* createStageInstance(const std::string& type, const nlohmann::json& params);
    void configureLogger(const nlohmann::json& loggerConfig);
};

#endif // ANALYSISPIPELINE_PIPELINE_H

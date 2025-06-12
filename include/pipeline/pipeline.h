#ifndef ANALYSISPIPELINE_PIPELINE_H
#define ANALYSISPIPELINE_PIPELINE_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <tbb/flow_graph.h>
#include <TObject.h>
#include "config_parser.h"
#include "base_stage.h" 

class Pipeline {
public:
    Pipeline() = default;
    bool buildFromConfig(const std::vector<StageConfig>& stages);
    void execute();

private:
    tbb::flow::graph graph_;

    // Store nodes by id
    std::map<std::string, std::unique_ptr<tbb::flow::continue_node<tbb::flow::continue_msg>>> nodes_;

    // Track incoming edge counts for each node id
    std::map<std::string, int> incomingCount_;

    // Cached start nodes (zero incoming edges), set once after buildFromConfig()
    std::vector<std::string> startNodes_;

    // Helper to create BaseStage instance
    BaseStage* createStageInstance(const std::string& type, const nlohmann::json& params);

    // Helper to find nodes with zero incoming edges (start nodes)
    std::vector<std::string> findStartNodes() const;
};

#endif // ANALYSISPIPELINE_PIPELINE_H

#ifndef ANALYSIS_PIPELINE_STAGE_NODE_H
#define ANALYSIS_PIPELINE_STAGE_NODE_H

#include <tbb/flow_graph.h>
#include "stages/base_stage.h"

class StageNode {
public:
    StageNode(tbb::flow::graph& g, BaseStage* stage);

    tbb::flow::continue_node<tbb::flow::continue_msg>& node();

    void try_put(tbb::flow::continue_msg msg);

private:
    BaseStage* stage_;  // owned externally, ROOT TObject lifecycle
    tbb::flow::continue_node<tbb::flow::continue_msg> node_;
};

#endif // ANALYSIS_PIPELINE_STAGE_NODE_H

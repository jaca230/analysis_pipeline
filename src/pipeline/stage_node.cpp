#include "stage_node.h"

StageNode::StageNode(tbb::flow::graph& g, BaseStage* stage)
    : stage_(stage)
    , node_(g,
        [this](const tbb::flow::continue_msg&) {
            this->stage_->Process();
        })
{}

tbb::flow::continue_node<tbb::flow::continue_msg>& StageNode::node() {
    return node_;
}

void StageNode::try_put(tbb::flow::continue_msg msg) {
    node_.try_put(msg);
}

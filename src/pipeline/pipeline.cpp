#include "pipeline.h"
#include <iostream>
#include <TClass.h>
#include <TROOT.h>

BaseStage* Pipeline::createStageInstance(const std::string& type, const nlohmann::json& params) {
    TClass* cls = TClass::GetClass(type.c_str());
    if (!cls) {
        std::cerr << "[Pipeline] ERROR: Class '" << type << "' not found in ROOT dictionary." << std::endl;
        return nullptr;
    }

    TObject* obj = static_cast<TObject*>(cls->New());
    if (!obj) {
        std::cerr << "[Pipeline] ERROR: Failed to instantiate class '" << type << "'." << std::endl;
        return nullptr;
    }

    BaseStage* stage = dynamic_cast<BaseStage*>(obj);
    if (!stage) {
        std::cerr << "[Pipeline] ERROR: Created object is not a BaseStage." << std::endl;
        delete obj;
        return nullptr;
    }

    stage->Init(params);
    return stage;
}

bool Pipeline::buildFromConfig(const std::vector<StageConfig>& stages) {
    graph_.reset();
    nodes_.clear();
    incomingCount_.clear();
    startNodes_.clear();

    for (const auto& sc : stages) {
        incomingCount_[sc.id] = 0;
    }

    for (const auto& sc : stages) {
        std::cout << "[Pipeline] Registering stage id: " << sc.id << " type: " << sc.type << std::endl;
        BaseStage* stageInstance = createStageInstance(sc.type, sc.parameters);
        if (!stageInstance) return false;

        auto node = std::make_unique<tbb::flow::continue_node<tbb::flow::continue_msg>>(graph_,
            [stageInstance](const tbb::flow::continue_msg&) {
                std::cout << "[Pipeline] Executing stage: " << stageInstance->Name() << std::endl;
                stageInstance->Process();
            });

        nodes_.emplace(sc.id, std::move(node));
    }

    std::cout << "[Pipeline] Connecting graph edges..." << std::endl;
    for (const auto& sc : stages) {
        auto fromNodeIt = nodes_.find(sc.id);
        if (fromNodeIt == nodes_.end()) {
            std::cerr << "[Pipeline] ERROR: Node id '" << sc.id << "' not found." << std::endl;
            return false;
        }
        for (const auto& nextId : sc.next) {
            auto toNodeIt = nodes_.find(nextId);
            if (toNodeIt == nodes_.end()) {
                std::cerr << "[Pipeline] ERROR: Next node id '" << nextId << "' not found." << std::endl;
                return false;
            }
            std::cout << "[Pipeline] Connecting " << sc.id << " -> " << nextId << std::endl;
            make_edge(*(fromNodeIt->second), *(toNodeIt->second));
            incomingCount_[nextId]++;
        }
    }

    // Cache start nodes (zero incoming edges)
    for (const auto& [id, count] : incomingCount_) {
        if (count == 0) {
            startNodes_.push_back(id);
        }
    }
    std::cout << "[Pipeline] Found " << startNodes_.size() << " start node(s)." << std::endl;

    return true;
}

void Pipeline::execute() {
    std::cout << "[Pipeline] Starting execution with " << startNodes_.size() << " start node(s)." << std::endl;

    for (const auto& id : startNodes_) {
        auto it = nodes_.find(id);
        if (it != nodes_.end()) {
            std::cout << "[Pipeline] Triggering start node: " << id << std::endl;
            it->second->try_put(tbb::flow::continue_msg());
        }
    }

    graph_.wait_for_all();
}

#include "pipeline/pipeline.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <TClass.h>
#include <TROOT.h>

Pipeline::Pipeline(std::shared_ptr<ConfigManager> configManager)
    : configManager_(std::move(configManager)) {}

std::shared_ptr<ConfigManager> Pipeline::getConfigManager() const {
    return configManager_;
}

PipelineDataProductManager& Pipeline::getDataProductManager() {
    return dataProductManager_;
}

void Pipeline::setConfigManager(std::shared_ptr<ConfigManager> configManager) {
    configManager_ = std::move(configManager);
}

BaseStage* Pipeline::createStageInstance(const std::string& type, const nlohmann::json& params) {
    TClass* cls = TClass::GetClass(type.c_str());
    if (!cls) {
        spdlog::error("[Pipeline] Class '{}' not found in ROOT dictionary.", type);
        return nullptr;
    }

    TObject* obj = static_cast<TObject*>(cls->New());
    if (!obj) {
        spdlog::error("[Pipeline] Failed to instantiate class '{}'.", type);
        return nullptr;
    }

    BaseStage* stage = dynamic_cast<BaseStage*>(obj);
    if (!stage) {
        spdlog::error("[Pipeline] Created object is not a BaseStage.");
        delete obj;
        return nullptr;
    }

    stage->Init(params, &dataProductManager_);
    return stage;
}

void Pipeline::configureLogger(const nlohmann::json& loggerConfig) {
    try {
        spdlog::level::level_enum level = spdlog::level::info;
        if (loggerConfig.contains("level")) {
            level = spdlog::level::from_str(loggerConfig["level"].get<std::string>());
        }

        std::vector<spdlog::sink_ptr> sinks;
        if (loggerConfig.value("console_enabled", true)) {
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        }

        if (loggerConfig.value("file_enabled", false)) {
            if (loggerConfig.contains("file_path")) {
                sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
                    loggerConfig["file_path"].get<std::string>(), true));
            }
        }

        std::string loggerName = loggerConfig.value("name", "pipeline_logger");
        auto logger = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());
        spdlog::set_default_logger(logger);

        logger->set_pattern(loggerConfig.value("pattern", "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v"));
        logger->set_level(level);

        spdlog::debug("[Pipeline] Logger '{}' configured.", loggerName);
    } catch (const std::exception& e) {
        spdlog::error("[Pipeline] Logger config error: {}", e.what());
    }
}

bool Pipeline::buildFromConfig() {
    if (!configManager_) {
        spdlog::error("[Pipeline] ConfigManager not set.");
        return false;
    }

    const auto& stagesConfig = configManager_->getPipelineStages();
    if (stagesConfig.empty()) {
        spdlog::error("[Pipeline] No pipeline stages loaded.");
        return false;
    }

    if (!configManager_->getLoggerConfig().raw.empty()) {
        configureLogger(configManager_->getLoggerConfig().raw);
    }

    graph_.reset();
    nodes_.clear();
    stages_.clear();
    incomingCount_.clear();
    startNodes_.clear();
    midas_unpackers_.clear();

    for (const auto& sc : stagesConfig) {
        incomingCount_[sc.id] = 0;
    }

    for (const auto& sc : stagesConfig) {
        spdlog::debug("[Pipeline] Registering stage id: {} type: {}", sc.id, sc.type);

        std::unique_ptr<BaseStage> stagePtr(createStageInstance(sc.type, sc.parameters));
        if (!stagePtr) return false;

        BaseStage* stageRaw = stagePtr.get();
        stages_[sc.id] = std::move(stagePtr);

        if (auto* unpacker = dynamic_cast<BaseMidasUnpackerStage*>(stageRaw)) {
            midas_unpackers_.push_back(unpacker);
        }

        auto node = std::make_unique<tbb::flow::continue_node<tbb::flow::continue_msg>>(graph_,
            [stageRaw](const tbb::flow::continue_msg&) {
                spdlog::debug("[Pipeline] Executing stage: {}", stageRaw->Name());
                stageRaw->Process();
            });

        nodes_[sc.id] = std::move(node);
    }

    for (const auto& sc : stagesConfig) {
        auto& fromNode = nodes_.at(sc.id);
        for (const auto& nextId : sc.next) {
            auto toIt = nodes_.find(nextId);
            if (toIt == nodes_.end()) {
                spdlog::error("[Pipeline] Invalid next id: {}", nextId);
                return false;
            }
            spdlog::debug("[Pipeline] Connecting {} -> {}", sc.id, nextId);
            make_edge(*fromNode, *toIt->second);
            incomingCount_[nextId]++;
        }
    }

    for (const auto& [id, count] : incomingCount_) {
        if (count == 0) {
            startNodes_.push_back(id);
        }
    }

    spdlog::debug("[Pipeline] Found {} start node(s).", startNodes_.size());
    return true;
}

void Pipeline::execute() {
    spdlog::debug("[Pipeline] Executing pipeline with {} start node(s).", startNodes_.size());
    for (const auto& id : startNodes_) {
        auto it = nodes_.find(id);
        if (it != nodes_.end()) {
            it->second->try_put(tbb::flow::continue_msg());
        }
    }
    graph_.wait_for_all();
}

void Pipeline::setCurrentEvent(const TMEvent& event) {
    for (auto* unpacker : midas_unpackers_) {
        if (unpacker) {
            unpacker->SetCurrentEvent(event);
        }
    }
}

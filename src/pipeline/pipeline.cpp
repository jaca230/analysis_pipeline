#include "pipeline.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <TClass.h>
#include <TROOT.h>

Pipeline::Pipeline(std::shared_ptr<ConfigManager> configManager)
    : configManager_(std::move(configManager))
{}

std::shared_ptr<ConfigManager> Pipeline::getConfigManager() const {
    return configManager_;
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

    stage->Init(params);
    return stage;
}

void Pipeline::configureLogger(const nlohmann::json& loggerConfig) {
    try {
        // Extract log level
        spdlog::level::level_enum level = spdlog::level::info;
        if (loggerConfig.contains("level")) {
            std::string levelStr = loggerConfig["level"].get<std::string>();
            level = spdlog::level::from_str(levelStr);
        }

        // Setup sinks
        std::vector<spdlog::sink_ptr> sinks;
        bool consoleEnabled = true;
        if (loggerConfig.contains("console_enabled")) {
            consoleEnabled = loggerConfig["console_enabled"].get<bool>();
        }
        if (consoleEnabled) {
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        }

        if (loggerConfig.contains("file_enabled") && loggerConfig["file_enabled"].get<bool>()) {
            if (loggerConfig.contains("file_path")) {
                std::string file_path = loggerConfig["file_path"].get<std::string>();
                sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(file_path, true));
            }
        }

        // Determine logger name, if provided and non-empty
        std::string loggerName = "pipeline_logger";
        if (loggerConfig.contains("name")) {
            const auto& nameFromConfig = loggerConfig["name"].get<std::string>();
            if (!nameFromConfig.empty()) {
                loggerName = nameFromConfig;
            }
        }

        auto logger = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());
        spdlog::set_default_logger(logger);

        // Now the presence or absence of [%n] in the pattern controls whether the name shows
        spdlog::debug("[Pipeline] Logger '{}' configured from JSON.", loggerName);

        // Pattern
        if (loggerConfig.contains("pattern")) {
            logger->set_pattern(loggerConfig["pattern"].get<std::string>());
        } else {
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        }

        logger->set_level(level);

    } catch (const std::exception& e) {
        spdlog::error("[Pipeline] Exception configuring logger: {}", e.what());
    }
}

bool Pipeline::buildFromConfig() {
    if (!configManager_) {
        spdlog::error("[Pipeline] ConfigManager not set.");
        return false;
    }

    const auto& stages = configManager_->getPipelineStages();
    if (stages.empty()) {
        spdlog::error("[Pipeline] No pipeline stages loaded from config.");
        return false;
    }

    if (!configManager_->getLoggerConfig().raw.empty()) {
        configureLogger(configManager_->getLoggerConfig().raw);
    }

    graph_.reset();
    nodes_.clear();
    incomingCount_.clear();
    startNodes_.clear();

    for (const auto& sc : stages) {
        incomingCount_[sc.id] = 0;
    }

    for (const auto& sc : stages) {
        spdlog::debug("[Pipeline] Registering stage id: {} type: {}", sc.id, sc.type);
        BaseStage* stageInstance = createStageInstance(sc.type, sc.parameters);
        if (!stageInstance) return false;

        auto node = std::make_unique<tbb::flow::continue_node<tbb::flow::continue_msg>>(graph_,
            [stageInstance](const tbb::flow::continue_msg&) {
                spdlog::debug("[Pipeline] Executing stage: {}", stageInstance->Name());
                stageInstance->Process();
            });

        nodes_.emplace(sc.id, std::move(node));
    }

    spdlog::debug("[Pipeline] Connecting graph edges...");
    for (const auto& sc : stages) {
        auto fromNodeIt = nodes_.find(sc.id);
        if (fromNodeIt == nodes_.end()) {
            spdlog::error("[Pipeline] Node id '{}' not found.", sc.id);
            return false;
        }
        for (const auto& nextId : sc.next) {
            auto toNodeIt = nodes_.find(nextId);
            if (toNodeIt == nodes_.end()) {
                spdlog::error("[Pipeline] Next node id '{}' not found.", nextId);
                return false;
            }
            spdlog::debug("[Pipeline] Connecting {} -> {}", sc.id, nextId);
            make_edge(*(fromNodeIt->second), *(toNodeIt->second));
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
    spdlog::debug("[Pipeline] Starting execution with {} start node(s).", startNodes_.size());

    for (const auto& id : startNodes_) {
        auto it = nodes_.find(id);
        if (it != nodes_.end()) {
            spdlog::debug("[Pipeline] Triggering start node: {}", id);
            it->second->try_put(tbb::flow::continue_msg());
        }
    }

    graph_.wait_for_all();
}

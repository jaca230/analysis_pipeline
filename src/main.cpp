#include <iostream>
#include <filesystem>
#include <memory>
#include "analysis_pipeline/config/config_manager.h"
#include "analysis_pipeline/pipeline/pipeline.h"
#include <nlohmann/json.hpp>

int main(int argc, char** argv) {
    // Locate config directory relative to this source file
    std::filesystem::path sourcePath = __FILE__;
    std::filesystem::path configDir = sourcePath.parent_path().parent_path() / "config";

    std::vector<std::string> configPaths = {
        (configDir / "plugins.json").string(),
        (configDir / "logger.json").string(),
        (configDir / "pipeline.json").string()
    };

    auto configManager = std::make_shared<ConfigManager>();
    if (!configManager->loadFiles(configPaths) || !configManager->validate()) {
        std::cerr << "Error: Failed to load or validate configuration." << std::endl;
        return 1;
    }

    Pipeline pipeline(configManager);
    if (!pipeline.buildFromConfig()) {
        std::cerr << "Error: Failed to build pipeline." << std::endl;
        return 1;
    }

    // Run the pipeline multiple times (e.g., 3 iterations)
    for (int i = 1; i <= 3; ++i) {
        pipeline.execute();

        auto jsonData = pipeline.getDataProductManager().serializeAll();
        std::cout << "\n[Pretty JSON Dump after run " << i << "]" << std::endl;
        std::cout << jsonData.dump(4) << std::endl;
    }

    // Clear data product manager once at the end
    pipeline.getDataProductManager().clear();

    return 0;
}

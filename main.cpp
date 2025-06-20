#include <iostream>
#include <filesystem>
#include <memory>
#include "config/config_manager.h"
#include "pipeline/pipeline.h"

int main(int argc, char** argv) {
    // Locate config directory relative to this source file
    std::filesystem::path sourcePath = __FILE__;
    std::filesystem::path configDir = sourcePath.parent_path() / "config";

    std::vector<std::string> configPaths = {
        (configDir / "logger.json").string(),
        (configDir / "pipeline.json").string()
    };

    // Create and load config manager
    auto configManager = std::make_shared<ConfigManager>();
    if (!configManager->loadFiles(configPaths)) {
        std::cerr << "Failed to load configuration files." << std::endl;
        return 1;
    }

    if (!configManager->validate()) {
        std::cerr << "Configuration validation failed." << std::endl;
        return 1;
    }

    // Construct pipeline
    Pipeline pipeline(configManager);

    if (!pipeline.buildFromConfig()) {
        std::cerr << "Failed to build pipeline from config." << std::endl;
        return 1;
    }

    // Run pipeline multiple times and print data products JSON
    const int runs = 5;
    for (int i = 0; i < runs; ++i) {
        pipeline.execute();

        // Get serialized JSON from the manager (thread-safe internally)
        nlohmann::json jsonData = pipeline.getDataProductManager().serializeAll();

        std::cout << "Run " << (i + 1) << " data products:" << std::endl;
        std::cout << jsonData.dump(4) << std::endl;
    }

    return 0;
}

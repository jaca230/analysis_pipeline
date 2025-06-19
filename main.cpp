#include <iostream>
#include <filesystem>
#include <memory>
#include "config/config_manager.h"
#include "pipeline/pipeline.h"

int main(int argc, char** argv) {
    // Determine default config file locations relative to source
    std::filesystem::path sourcePath = __FILE__;
    std::filesystem::path configDir = sourcePath.parent_path() / "config";

    // You could also allow command line overrides here if needed
    std::vector<std::string> configPaths = {
        (configDir / "logger.json").string(),
        (configDir / "pipeline.json").string()
    };

    // Create shared ConfigManager instance
    auto configManager = std::make_shared<ConfigManager>();

    // Load configuration files (includes build)
    if (!configManager->loadFiles(configPaths)) {
        std::cerr << "Error: Failed to load and build configuration." << std::endl;
        return 1;
    }

    // Optional: validate configuration
    if (!configManager->validate()) {
        std::cerr << "Error: Configuration validation failed." << std::endl;
        return 1;
    }

    // Build and run pipeline
    Pipeline pipeline(configManager);
    if (!pipeline.buildFromConfig()) {
        std::cerr << "Error: Failed to build pipeline." << std::endl;
        return 1;
    }

    pipeline.execute();

    return 0;
}

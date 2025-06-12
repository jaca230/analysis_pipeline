#include <iostream>
#include <string>
#include <filesystem>

#include "config_parser.h"
#include "pipeline.h"

int main(int argc, char** argv) {
    // Get the directory of the source file (compile-time constant)
    std::filesystem::path sourcePath = __FILE__;  // e.g., /home/user/project/src/main.cpp
    std::filesystem::path configPath = sourcePath.parent_path().parent_path() / "config" / "pipeline.json";

    std::cout << "Using default config at: " << configPath << "\n";

    ConfigParser parser;
    if (!parser.parseFile(configPath.string())) {
        std::cerr << "Failed to parse pipeline config.\n";
        return 1;
    }

    Pipeline pipeline;
    if (!pipeline.buildFromConfig(parser.getStages())) {
        std::cerr << "Failed to build pipeline from config.\n";
        return 1;
    }

    pipeline.execute();

    std::cout << "Pipeline execution complete.\n";
    return 0;
}

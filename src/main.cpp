#include <iostream>
#include <filesystem>
#include <memory>
#include <ctime>
#include "config/config_manager.h"
#include "pipeline/pipeline.h"
#include "midasio.h"
#include <nlohmann/json.hpp>

TMEvent makeDummyEvent(int event_id, int serial, const std::string& payload) {
    TMEvent evt;
    evt.Init(event_id, serial, 0xABCD, static_cast<uint32_t>(std::time(nullptr)));
    evt.AddBank("DUMY", TID_STRING, payload.c_str(), payload.size() + 1);
    evt.FindAllBanks();
    return evt;
}

void runEventAndPrint(Pipeline& pipeline, const TMEvent& evt, const std::string& label) {
    pipeline.setCurrentEvent(evt);
    pipeline.execute();

    auto jsonData = pipeline.getDataProductManager().serializeAll();
    std::cout << "\n[Pretty JSON Dump from '" << label << "']" << std::endl;
    std::cout << jsonData.dump(4) << std::endl;
}

int main(int argc, char** argv) {
    // Locate config directory
    std::filesystem::path sourcePath = __FILE__;
    std::filesystem::path configDir = sourcePath.parent_path().parent_path() / "config";

    std::vector<std::string> configPaths = {
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

    // Run pipeline with multiple dummy events and print results
    runEventAndPrint(pipeline, makeDummyEvent(1, 100, "FIRST_PAYLOAD"), "Event 1");
    runEventAndPrint(pipeline, makeDummyEvent(2, 200, "SECOND_PAYLOAD"), "Event 2");
    runEventAndPrint(pipeline, makeDummyEvent(3, 300, "THIRD_PAYLOAD"), "Event 3");

    return 0;
}

#include <iostream>
#include <filesystem>
#include <memory>
#include <cstring>
#include <ctime>
#include <mutex>

#include "config/config_manager.h"
#include "pipeline/pipeline.h"
#include "midasio.h"

#include <TTree.h>
#include <nlohmann/json.hpp>

void printEventJsonFromTree(TTree* tree, const std::string& label) {
    std::cout << "\n[Pretty JSON Dump from '" << label << "']" << std::endl;

    std::string* json_str_ptr = nullptr;
    tree->SetBranchAddress("event_json", &json_str_ptr);

    for (Long64_t i = 0; i < tree->GetEntries(); ++i) {
        tree->GetEntry(i);
        try {
            if (json_str_ptr) {
                auto parsed = nlohmann::json::parse(*json_str_ptr);
                std::cout << "=== Entry " << i << " ===" << std::endl;
                std::cout << parsed.dump(4) << std::endl << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing JSON in entry " << i << ": " << e.what() << std::endl;
        }
    }
}

TMEvent makeDummyEvent(int event_id, int serial, const std::string& payload) {
    TMEvent evt;
    evt.Init(event_id, serial, 0xABCD, static_cast<uint32_t>(std::time(nullptr)));

    evt.AddBank("DUMY", TID_STRING, payload.c_str(), payload.size() + 1);
    evt.FindAllBanks();
    return evt;
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

    // Tree 1: Run twice with 2 different dummy events
    TTree* tree1 = new TTree("events", "Dummy MIDAS Events");
    pipeline.setTree(tree1);

    TMEvent event1 = makeDummyEvent(1, 100, "FIRST_PAYLOAD");
    pipeline.setCurrentEvent(event1);
    pipeline.execute();

    TMEvent event2 = makeDummyEvent(2, 200, "SECOND_PAYLOAD");
    pipeline.setCurrentEvent(event2);
    pipeline.execute();

    // Tree 2: Run once with a fresh event
    TTree* tree2 = new TTree("events", "Fresh Dummy MIDAS Events");
    pipeline.setTree(tree2);

    TMEvent event3 = makeDummyEvent(3, 300, "THIRD_PAYLOAD");
    pipeline.setCurrentEvent(event3);
    pipeline.execute();

    // Print JSON contents
    printEventJsonFromTree(tree1, "tree1 (2 entries)");
    printEventJsonFromTree(tree2, "tree2 (1 entry)");

    return 0;
}

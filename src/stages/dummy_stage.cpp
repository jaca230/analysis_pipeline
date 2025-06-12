#include "dummy_stage.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>
#include <iomanip>  // for put_time
#include <ctime>    // for std::localtime

DummyStage::DummyStage() = default;
DummyStage::~DummyStage() = default;

void DummyStage::OnInit() {
    spdlog::debug("[{}] OnInit called with config: {}", Name(), parameters_.dump());
}

void DummyStage::Process() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    // Format time to string first
    std::stringstream ss_start;
    ss_start << std::put_time(std::localtime(&now_c), "%F %T");

    spdlog::info("[{}] Process started at {}.{:03} with config: {}",
                 Name(),
                 ss_start.str(),
                 static_cast<int>(now_ms.count()),
                 parameters_.dump());

    if (parameters_.contains("sleep_ms")) {
        int sleep_time = parameters_["sleep_ms"].get<int>();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    }

    now = std::chrono::system_clock::now();
    now_c = std::chrono::system_clock::to_time_t(now);
    now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    // Format time to string again
    std::stringstream ss_end;
    ss_end << std::put_time(std::localtime(&now_c), "%F %T");

    spdlog::info("[{}] Process finished at {}.{:03} with config: {}",
                 Name(),
                 ss_end.str(),
                 static_cast<int>(now_ms.count()),
                 parameters_.dump());
}


std::string DummyStage::Name() const {
    return "DummyStage";
}

ClassImp(DummyStage)

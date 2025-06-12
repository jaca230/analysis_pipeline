#include "dummy_stage.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>  // for put_time
#include <ctime>    // for std::localtime

DummyStage::DummyStage() = default;
DummyStage::~DummyStage() = default;

void DummyStage::OnInit() {
    std::cout << "[DummyStage] OnInit called with config: " << parameters_.dump() << std::endl;
}


void DummyStage::Process() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::cout << "[" << Name() << "] Process started at "
              << std::put_time(std::localtime(&now_c), "%F %T") << "."
              << std::setfill('0') << std::setw(3) << now_ms.count()
              << " with config: " << parameters_.dump() << std::endl;

    if (parameters_.contains("sleep_ms")) {
        int sleep_time = parameters_["sleep_ms"].get<int>();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    }

    now = std::chrono::system_clock::now();
    now_c = std::chrono::system_clock::to_time_t(now);
    now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::cout << "[" << Name() << "] Process finished at "
              << std::put_time(std::localtime(&now_c), "%F %T") << "."
              << std::setfill('0') << std::setw(3) << now_ms.count()
              << " with config: " << parameters_.dump() << std::endl;
}

std::string DummyStage::Name() const {
    return "DummyStage";
}

ClassImp(DummyStage)

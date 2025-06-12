#include "dummy_stage.h"
#include <iostream>

DummyStage::DummyStage() = default;
DummyStage::~DummyStage() = default;

void DummyStage::OnInit() {
    std::cout << "[DummyStage] OnInit called with config: " << parameters_.dump() << std::endl;
}

void DummyStage::Process() {
    std::cout << "[DummyStage] Process called with config: " << parameters_.dump() << std::endl;
}

std::string DummyStage::Name() const {
    return "DummyStage";
}

ClassImp(DummyStage)

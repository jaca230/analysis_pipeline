#include "base_stage.h"

BaseStage::BaseStage() = default;
BaseStage::~BaseStage() = default;

void BaseStage::Init(const nlohmann::json& parameters) {
    parameters_ = parameters;
    OnInit();  // Let derived classes do their thing
}

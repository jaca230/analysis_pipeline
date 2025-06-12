#ifndef ANALYSIS_PIPELINE_STAGES_DUMMYSTAGE_H
#define ANALYSIS_PIPELINE_STAGES_DUMMYSTAGE_H

#include "base_stage.h"
#include <nlohmann/json.hpp>

class DummyStage : public BaseStage {
public:
    DummyStage();
    ~DummyStage() override;

    void Process() override;
    std::string Name() const override;

protected:
    void OnInit() override;  // override this instead of Init
};


#endif // ANALYSIS_PIPELINE_STAGES_DUMMYSTAGE_H

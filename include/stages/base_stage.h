#ifndef ANALYSIS_PIPELINE_STAGES_BASESTAGE_H
#define ANALYSIS_PIPELINE_STAGES_BASESTAGE_H

#include <TObject.h>
#include <string>
#include <nlohmann/json.hpp>

class BaseStage : public TObject {
public:
    BaseStage();
    virtual ~BaseStage();

    void Init(const nlohmann::json& parameters);  // Final, not meant to be overridden
    virtual void Process() = 0;
    virtual std::string Name() const = 0;

protected:
    virtual void OnInit() {}  // Hook for derived classes

    nlohmann::json parameters_;

    ClassDef(BaseStage, 1)
};


#endif // ANALYSIS_PIPELINE_STAGES_BASESTAGE_H

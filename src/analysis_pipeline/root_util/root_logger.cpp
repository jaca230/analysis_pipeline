#include "analysis_pipeline/root_util/root_logger.h"

#include <TError.h>  // for SetErrorHandler and error levels
#include <spdlog/spdlog.h>
#include <cstdlib>   // for std::abort

namespace {
void CustomROOTErrorHandler(Int_t level, Bool_t abort, const char* location, const char* msg) {
    // Custom error handler for ROOT errors
    // I don't care about any output unless it's fatal
    if (level >= kFatal) {
        spdlog::error("[ROOT][{}] {}", location, msg);
    } else if (level >= kError) {
        spdlog::debug("[ROOT][{}] {}", location, msg);
    } else if (level >= kWarning) {
        spdlog::debug("[ROOT][{}] {}", location, msg);
    } else if (level >= kInfo) {
        spdlog::debug("[ROOT][{}] {}", location, msg);
    } else {
        spdlog::debug("[ROOT][{}] {}", location, msg);
    }

    if (abort) {
        spdlog::critical("[ROOT] aborting after error.");
        std::abort();
    }
}
} // anonymous namespace

RootLogger& RootLogger::instance() {
    static RootLogger instance;
    return instance;
}

RootLogger::RootLogger() {
    previousHandler_ = ::SetErrorHandler(CustomROOTErrorHandler);
}

RootLogger::~RootLogger() {
    ::SetErrorHandler(previousHandler_);
}

cmake_minimum_required(VERSION 3.14)
project(AnalysisPipeline VERSION 0.1 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find ROOT
find_package(ROOT REQUIRED COMPONENTS Tree Core Hist RIO)
if(NOT ROOT_FOUND)
    message(FATAL_ERROR "ROOT not found. Make sure to source ROOT (e.g. 'source thisroot.sh').")
endif()
include(${ROOT_USE_FILE})

# Find Intel TBB
find_package(TBB REQUIRED)
if(NOT TBB_FOUND)
    message(FATAL_ERROR "TBB not found. On Ubuntu, run: sudo apt install libtbb-dev")
endif()

# FetchContent for spdlog and nlohmann/json
include(FetchContent)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# Add external submodule
add_subdirectory(external/analysis_pipeline_stages)

# Source files for main executable
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

# Main executable target
add_executable(analysis_pipeline src/main.cpp ${SRC_FILES})

# Include our own top-level include/ and submodule include path
target_include_directories(analysis_pipeline PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/external/analysis_pipeline_stages/include
)

# Link main executable to libraries
target_link_libraries(analysis_pipeline PRIVATE
  ROOT::Core ROOT::RIO ROOT::Tree
  TBB::tbb
  analysis_pipeline_stages
  spdlog::spdlog
  nlohmann_json::nlohmann_json
)

# Install rule
install(TARGETS analysis_pipeline
        RUNTIME DESTINATION bin)

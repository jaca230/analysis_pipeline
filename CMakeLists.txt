cmake_minimum_required(VERSION 3.14)
project(AnalysisPipeline VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Check MIDASSYS environment variable
if(NOT DEFINED ENV{MIDASSYS})
  message(FATAL_ERROR
    "Environment variable MIDASSYS not set.\n"
    "Please source your MIDAS environment or run the detect_environment.sh script:\n"
    "  ./scripts/environment/detect_environment.sh\n"
    "  source ./scripts/environment/.env"
  )
endif()

set(MIDASSYS_INCLUDE_DIRS
  $ENV{MIDASSYS}/include
  $ENV{MIDASSYS}/midasio
)
set(MIDASSYS_LIB_DIR $ENV{MIDASSYS}/lib)

# Find ROOT
find_package(ROOT REQUIRED COMPONENTS Tree Core Hist RIO)
if(NOT ROOT_FOUND)
  message(FATAL_ERROR "ROOT not found. Make sure to source ROOT (e.g. 'source thisroot.sh').")
endif()
include(${ROOT_USE_FILE})

# Find TBB
find_package(TBB REQUIRED)
if(NOT TBB_FOUND)
  message(FATAL_ERROR "TBB not found. On Ubuntu, try: sudo apt install libtbb-dev")
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

# Add external submodule for pipeline stages
add_subdirectory(external/analysis_pipeline_stages)

# Source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

# Main executable target
add_executable(analysis_pipeline
  src/main.cpp
  ${SRC_FILES}
)

# Include your own project headers (top-level include)
target_include_directories(analysis_pipeline PRIVATE
  ${CMAKE_SOURCE_DIR}/include
)

# Add MIDAS include dirs to target
target_include_directories(analysis_pipeline PRIVATE
  ${MIDASSYS_INCLUDE_DIRS}
)

find_library(MIDAS_LIB
  NAMES midas
  PATHS ${MIDASSYS_LIB_DIR}
  NO_DEFAULT_PATH
)

# Link libraries
target_link_libraries(analysis_pipeline PRIVATE
  ROOT::Core ROOT::RIO ROOT::Tree
  TBB::tbb
  analysis_pipeline_stages
  spdlog::spdlog
  nlohmann_json::nlohmann_json
  ${MIDAS_LIB}
)

# Install rules
install(TARGETS analysis_pipeline
  RUNTIME DESTINATION bin
)

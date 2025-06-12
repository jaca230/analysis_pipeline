cmake_minimum_required(VERSION 3.14)
project(AnalysisPipeline VERSION 0.1 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include paths for your own project headers
file(GLOB_RECURSE ALL_INCLUDE_PATHS LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/include/*)
foreach(path ${ALL_INCLUDE_PATHS})
    if(IS_DIRECTORY ${path})
        include_directories(${path})
    endif()
endforeach()

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

# Add external submodule library
add_subdirectory(external/analysis_pipeline_stages)

# Source files for main executable
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

# Main executable target
add_executable(analysis_pipeline src/main.cpp ${SRC_FILES})

# Link main executable to libraries
target_link_libraries(analysis_pipeline PRIVATE
  ${ROOT_LIBRARIES}
  TBB::tbb
  analysis_pipeline_stages     # <-- submodule library target
  spdlog::spdlog
  nlohmann_json::nlohmann_json
)

# Include submodule headers (if not already included by the submodule's CMake)
target_include_directories(analysis_pipeline PRIVATE external/analysis_pipeline_stages/include)

# Install rules
install(TARGETS analysis_pipeline
        RUNTIME DESTINATION bin)

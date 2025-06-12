cmake_minimum_required(VERSION 3.14)
project(AnalysisPipeline VERSION 0.1 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include paths
file(GLOB_RECURSE ALL_INCLUDE_PATHS LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/include/*)
foreach(path ${ALL_INCLUDE_PATHS})
    if(IS_DIRECTORY ${path})
        include_directories(${path})
    endif()
endforeach()

# Source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS include/**/*.h)

# ROOT
find_package(ROOT REQUIRED COMPONENTS Tree Core Hist RIO)
if(NOT ROOT_FOUND)
    message(FATAL_ERROR "ROOT not found. Make sure to source ROOT (e.g. 'source thisroot.sh').")
endif()
include(${ROOT_USE_FILE})

# Intel TBB
find_package(TBB REQUIRED)
if(NOT TBB_FOUND)
    message(FATAL_ERROR "TBB not found. On Ubuntu, run: sudo apt install libtbb-dev")
endif()

# FetchContent module
include(FetchContent)

# spdlog
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

# nlohmann/json
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# ROOT dictionary generation for stages
set(STAGE_HEADERS
  ${CMAKE_SOURCE_DIR}/include/stages/base_stage.h
  ${CMAKE_SOURCE_DIR}/include/stages/dummy_stage.h
)

ROOT_GENERATE_DICTIONARY(G__Stages
  ${STAGE_HEADERS}
  LINKDEF ${CMAKE_SOURCE_DIR}/include/stages/LinkDef.h
)

# Create shared library for stages (including dictionary)
add_library(stages_dict SHARED
  ${CMAKE_SOURCE_DIR}/src/stages/base_stage.cpp
  ${CMAKE_SOURCE_DIR}/src/stages/dummy_stage.cpp
  G__Stages.cxx
)

target_include_directories(stages_dict PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(stages_dict PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree
  spdlog::spdlog
)

# Main executable target
add_executable(analysis_pipeline src/main.cpp ${SRC_FILES})

target_link_libraries(analysis_pipeline PRIVATE
  ${ROOT_LIBRARIES}
  TBB::tbb
  stages_dict
  spdlog::spdlog
  nlohmann_json::nlohmann_json
)

# Install rules
install(TARGETS analysis_pipeline stages_dict
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

cmake_minimum_required(VERSION 3.14)
project(AnalysisPipeline VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories to local bin/ and lib/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

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

# Add the submodule - handles ROOT dictionary generation and builds the stages library
add_subdirectory(external/analysis_pipeline_stages)

# Include directories for main project
include_directories(
  include
  ${MIDASSYS_INCLUDE_DIRS}
)

# Split main.cpp from the rest
set(MAIN_EXECUTABLE_SRC src/main.cpp)
file(GLOB_RECURSE LIB_SRC CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM LIB_SRC ${CMAKE_SOURCE_DIR}/src/main.cpp)

# Build shared library: libanalysis_pipeline.so
add_library(analysis_pipeline SHARED ${LIB_SRC})
target_link_libraries(analysis_pipeline
  analysis_pipeline_stages
  ROOT::Core ROOT::RIO ROOT::Tree
  TBB::tbb
  spdlog::spdlog
  nlohmann_json::nlohmann_json
)

# Build main executable: analysis_pipeline (links against the above lib)
add_executable(analysis_pipeline_exec ${MAIN_EXECUTABLE_SRC})
target_link_libraries(analysis_pipeline_exec analysis_pipeline)

# MIDAS linking (needed for runtime)
link_directories(${MIDASSYS_LIB_DIR})

# Optionally rename executable to `analysis_pipeline`
set_target_properties(analysis_pipeline_exec PROPERTIES OUTPUT_NAME analysis_pipeline)

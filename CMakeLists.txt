cmake_minimum_required(VERSION 3.14)

project(AnalysisPipeline VERSION 0.1 LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_EXAMPLE_PLUGIN "Build the example plugin if available" ON)

# Suppress false-positive GCC warnings when top-level
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-stringop-overflow)
  endif()
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# --------------------- CPM Setup ---------------------
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)

# Use externally-defined versions if set, else fallback
set(CPM_SPDLOG_VERSION "1.13.0" CACHE STRING "spdlog version")
set(CPM_NLOHMANN_JSON_VERSION "3.11.3" CACHE STRING "nlohmann_json version")

# --------------------- spdlog ---------------------
CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  VERSION ${CPM_SPDLOG_VERSION}
  OPTIONS
    "SPDLOG_BUILD_EXAMPLES OFF"
    "SPDLOG_BUILD_TESTS OFF"
    "SPDLOG_FMT_EXTERNAL OFF"
  DOWNLOAD_ONLY YES
)

if(spdlog_ADDED)
  add_library(spdlog_header_only INTERFACE)
  target_include_directories(spdlog_header_only INTERFACE
    $<BUILD_INTERFACE:${spdlog_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
  add_library(spdlog::spdlog ALIAS spdlog_header_only)
endif()

# --------------------- nlohmann_json ---------------------
CPMAddPackage(
  NAME nlohmann_json
  GITHUB_REPOSITORY nlohmann/json
  VERSION ${CPM_NLOHMANN_JSON_VERSION}
  DOWNLOAD_ONLY YES
)

if(nlohmann_json_ADDED)
  add_library(nlohmann_json_header_only INTERFACE)
  target_include_directories(nlohmann_json_header_only INTERFACE
    $<BUILD_INTERFACE:${nlohmann_json_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
  add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json_header_only)
endif()

# System dependencies
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist)
include(${ROOT_USE_FILE})

find_package(TBB REQUIRED)
find_package(AnalysisPipelineStagesCore REQUIRED)

# Gather source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(FILTER SRC_FILES EXCLUDE REGEX "main\\.cpp$")
set(MAIN_EXECUTABLE_SRC src/main.cpp)

# Create shared library
add_library(analysis_pipeline SHARED ${SRC_FILES})
add_library(AnalysisPipeline::analysis_pipeline ALIAS analysis_pipeline)

set_target_properties(analysis_pipeline PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 0
)

target_include_directories(analysis_pipeline PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(analysis_pipeline PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist
  TBB::tbb
  AnalysisPipelineStagesCore::analysis_pipeline_stages_core
  spdlog_header_only
  nlohmann_json_header_only
)

# Executable
add_executable(analysis_pipeline_exec ${MAIN_EXECUTABLE_SRC})
target_link_libraries(analysis_pipeline_exec PRIVATE analysis_pipeline)
set_target_properties(analysis_pipeline_exec PROPERTIES OUTPUT_NAME analysis_pipeline)

# Install/export logic only if top-level project
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)

  install(TARGETS analysis_pipeline
    EXPORT AnalysisPipelineTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline
  )

  install(TARGETS spdlog_header_only nlohmann_json_header_only
    EXPORT AnalysisPipelineTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline
  )

  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline
  )

  install(EXPORT AnalysisPipelineTargets
    FILE AnalysisPipelineTargets.cmake
    NAMESPACE AnalysisPipeline::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipeline
  )

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipeline
  )

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipeline
  )

  export(PACKAGE AnalysisPipeline)
endif()

# Optional example plugin
set(EXAMPLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples/example_plugin")

if(BUILD_EXAMPLE_PLUGIN)
  if(EXISTS "${EXAMPLES_DIR}/CMakeLists.txt")
    message(STATUS "Found example_plugin submodule and BUILD_EXAMPLE_PLUGIN=ON, adding as subdirectory.")
    add_subdirectory(${EXAMPLES_DIR})
  else()
    message(WARNING "BUILD_EXAMPLE_PLUGIN=ON but example plugin not found at ${EXAMPLES_DIR}")
  endif()
else()
  message(STATUS "Example plugin disabled (BUILD_EXAMPLE_PLUGIN=OFF), skipping example_plugin build.")
endif()

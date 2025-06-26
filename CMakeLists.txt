cmake_minimum_required(VERSION 3.14)

project(AnalysisPipeline VERSION 0.1 LANGUAGES CXX)

# Position independent code (for shared libs)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add option to control example plugin building
option(BUILD_EXAMPLE_PLUGIN "Build the example plugin if available" ON)

# Find ROOT
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist)
include(${ROOT_USE_FILE})

# Find TBB
find_package(TBB REQUIRED)

# Find spdlog
find_package(spdlog REQUIRED)

# Find nlohmann_json
find_package(nlohmann_json REQUIRED)

# Use AnalysisPipelineStagesCore instead of AnalysisPipelineStages
find_package(AnalysisPipelineStagesCore REQUIRED)

# Gather source files, exclude main.cpp
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(FILTER SRC_FILES EXCLUDE REGEX "main\\.cpp$")

# Main executable source
set(MAIN_EXECUTABLE_SRC src/main.cpp)

# Create shared library for analysis pipeline code
add_library(analysis_pipeline SHARED ${SRC_FILES})

set_target_properties(analysis_pipeline PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 0
)

target_compile_features(analysis_pipeline PUBLIC cxx_std_17)

target_include_directories(analysis_pipeline PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline>
)

target_link_libraries(analysis_pipeline PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist
  TBB::tbb
  spdlog::spdlog
  nlohmann_json::nlohmann_json
  AnalysisPipelineStagesCore::analysis_pipeline_stages_core
)

add_library(AnalysisPipeline::analysis_pipeline ALIAS analysis_pipeline)

add_executable(analysis_pipeline_exec ${MAIN_EXECUTABLE_SRC})
target_link_libraries(analysis_pipeline_exec PRIVATE analysis_pipeline)
set_target_properties(analysis_pipeline_exec PROPERTIES OUTPUT_NAME analysis_pipeline)

# Installation and export only for this package
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  install(TARGETS analysis_pipeline
    EXPORT AnalysisPipelineTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline
  )

  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline
  )

  install(EXPORT AnalysisPipelineTargets
    FILE AnalysisPipelineTargets.cmake
    NAMESPACE AnalysisPipeline::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipeline
    EXPORT_LINK_INTERFACE_LIBRARIES
  )

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipeline
  )

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipeline
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipeline
  )

  export(PACKAGE AnalysisPipeline)
endif()

# Optionally build the example plugin if it exists and option is enabled
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

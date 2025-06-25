cmake_minimum_required(VERSION 3.14)
project(AnalysisPipeline VERSION 0.1 LANGUAGES CXX)

# Enforce PIC globally (good for shared libs and static libs usable in shared libs)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Require C++17 explicitly
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # Disable compiler-specific extensions for portability

# Add option to control example plugin building
option(BUILD_EXAMPLE_PLUGIN "Build the example plugin if available" ON)

# Find ROOT - required components only
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist)
if(NOT ROOT_FOUND)
  message(FATAL_ERROR "ROOT not found. Please source thisroot.sh before configuring.")
endif()
include(${ROOT_USE_FILE})

# Find TBB - required
find_package(TBB REQUIRED)

# Find spdlog (header-only library but provides CMake config)
find_package(spdlog REQUIRED)

# Find nlohmann_json (header-only library)
find_package(nlohmann_json REQUIRED)

# Find analysis_pipeline_stages (assumed installed or findable via CMAKE_PREFIX_PATH)
find_package(AnalysisPipelineStages REQUIRED)

# Collect sources, excluding main.cpp explicitly (better than regex, but regex is fine here too)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(FILTER SRC_FILES EXCLUDE REGEX "main\\.cpp$")

set(MAIN_EXECUTABLE_SRC main.cpp)

# Create shared library for the pipeline
add_library(analysis_pipeline SHARED ${SRC_FILES})

# Set version and SOVERSION for compatibility control
set_target_properties(analysis_pipeline PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 0
)

# Use modern compile feature interface
target_compile_features(analysis_pipeline PUBLIC cxx_std_17)

# Interface include directories for users of the library
target_include_directories(analysis_pipeline
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline>
)

# Link dependencies publicly
target_link_libraries(analysis_pipeline PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist
  TBB::tbb
  spdlog::spdlog
  nlohmann_json::nlohmann_json
  AnalysisPipelineStages::analysis_pipeline_stages
)

# Provide an alias target for easier usage by downstream projects
add_library(AnalysisPipeline::analysis_pipeline ALIAS analysis_pipeline)

# Main executable linking to the pipeline library
add_executable(analysis_pipeline_exec ${MAIN_EXECUTABLE_SRC})
target_link_libraries(analysis_pipeline_exec PRIVATE analysis_pipeline)
set_target_properties(analysis_pipeline_exec PROPERTIES OUTPUT_NAME analysis_pipeline)

# Installation logic (only at top-level project)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  # Install library targets
  install(TARGETS analysis_pipeline
    EXPORT AnalysisPipelineTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline
  )

  # Install headers
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
  )
  
  # Export targets for find_package support
  install(EXPORT AnalysisPipelineTargets
    FILE AnalysisPipelineTargets.cmake
    NAMESPACE AnalysisPipeline::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipeline
  )

  # Package version config file
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipeline
  )

  # Package config file
  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipeline
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipeline
  )

  # Export the package for find_package()
  export(PACKAGE AnalysisPipeline)
endif()


# Optionally build the example plugin if it exists
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


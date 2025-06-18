cmake_minimum_required(VERSION 3.14)
project(AnalysisPipeline VERSION 0.1 LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Set up C++17 properly
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find ROOT
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist)
if(NOT ROOT_FOUND)
  message(FATAL_ERROR "ROOT not found. Make sure to source thisroot.sh")
endif()
include(${ROOT_USE_FILE})

# Find TBB
find_package(TBB REQUIRED)

# Add external stages library
add_subdirectory(external/analysis_pipeline_stages)

# Gather sources
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(FILTER SRC_FILES EXCLUDE REGEX "main\\.cpp$")
set(MAIN_EXECUTABLE_SRC src/main.cpp)

# Create shared library
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
  analysis_pipeline_stages
)

add_library(AnalysisPipeline::analysis_pipeline ALIAS analysis_pipeline)

# Main executable
add_executable(analysis_pipeline_exec ${MAIN_EXECUTABLE_SRC})
target_link_libraries(analysis_pipeline_exec PRIVATE analysis_pipeline)
set_target_properties(analysis_pipeline_exec PROPERTIES OUTPUT_NAME analysis_pipeline)

# Install logic
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  install(TARGETS analysis_pipeline analysis_pipeline_stages
    EXPORT AnalysisPipelineTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline
  )

  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline
  )

  # Install header-only deps into export set
  if(TARGET spdlog::spdlog)
    install(TARGETS spdlog
      EXPORT AnalysisPipelineTargets
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/spdlog
    )
  endif()

  if(TARGET nlohmann_json::nlohmann_json)
    install(TARGETS nlohmann_json
      EXPORT AnalysisPipelineTargets
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nlohmann_json
    )
  endif()

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

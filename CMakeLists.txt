cmake_minimum_required(VERSION 3.14)

project(AnalysisPipeline VERSION 0.1 LANGUAGES CXX)

# Position independent code (for shared libs)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# Enable compile_commands.json generation for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Require MIDASSYS environment variable
if(NOT DEFINED ENV{MIDASSYS})
  message(FATAL_ERROR
    "Environment variable MIDASSYS not set.\n"
    "Please source your MIDAS environment or run the detect_environment.sh script:\n"
    "  ./scripts/environment/detect_environment.sh\n"
    "  source ./scripts/environment/.env"
  )
endif()

# MIDAS include and lib directories
set(MIDASSYS_INCLUDE_DIRS
  $ENV{MIDASSYS}/include
  $ENV{MIDASSYS}/midasio
)
set(MIDASSYS_LIB_DIR $ENV{MIDASSYS}/lib)

# Find ROOT
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist)
if(NOT ROOT_FOUND)
  message(FATAL_ERROR "ROOT not found. Make sure to source thisroot.sh")
endif()
include(${ROOT_USE_FILE})

# Find TBB
find_package(TBB REQUIRED)
if(NOT TBB_FOUND)
  message(FATAL_ERROR "TBB not found. On Ubuntu, try: sudo apt install libtbb-dev")
endif()

# Add external stages library (assumed to handle its own dependencies)
add_subdirectory(external/analysis_pipeline_stages)

# Gather source files (exclude main.cpp)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(FILTER SRC_FILES EXCLUDE REGEX "main\\.cpp$")
set(MAIN_EXECUTABLE_SRC src/main.cpp)

# Create shared library for main pipeline code
add_library(analysis_pipeline SHARED ${SRC_FILES})

# Set version properties
set_target_properties(analysis_pipeline PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 0
)

target_compile_features(analysis_pipeline PUBLIC cxx_std_17)

# Add includes: your include dir + MIDAS includes (for compile)
target_include_directories(analysis_pipeline PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${MIDASSYS_INCLUDE_DIRS}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline>
  $<INSTALL_INTERFACE:include/midas>
)

# Link libs: ROOT, TBB, dependencies, MIDAS libs, and analysis_pipeline_stages
target_link_libraries(analysis_pipeline PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist
  TBB::tbb
  spdlog::spdlog
  nlohmann_json::nlohmann_json
  analysis_pipeline_stages
  # MIDAS libs (static or shared as needed)
  ${MIDASSYS_LIB_DIR}/libmfe.a
  ${MIDASSYS_LIB_DIR}/libmidas.a
)

# Alias target for convenience
add_library(AnalysisPipeline::analysis_pipeline ALIAS analysis_pipeline)

# Build main executable
add_executable(analysis_pipeline_exec ${MAIN_EXECUTABLE_SRC})
target_link_libraries(analysis_pipeline_exec PRIVATE analysis_pipeline)
set_target_properties(analysis_pipeline_exec PROPERTIES OUTPUT_NAME analysis_pipeline)

# Link directories for MIDAS libs (optional but recommended)
link_directories(${MIDASSYS_LIB_DIR})

# Install rules and export (only if top project)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)

  install(TARGETS analysis_pipeline analysis_pipeline_stages
    EXPORT AnalysisPipelineTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline
  )

  # Install headers
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline
  )

  # Also install MIDAS headers for consumers (optional)
  install(DIRECTORY $ENV{MIDASSYS}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/midas
  )
  install(DIRECTORY $ENV{MIDASSYS}/midasio/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/midas
  )

  # Install header-only dependencies in export set
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

  # Export targets for find_package and usage
  install(EXPORT AnalysisPipelineTargets
    FILE AnalysisPipelineTargets.cmake
    NAMESPACE AnalysisPipeline::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipeline
    EXPORT_LINK_INTERFACE_LIBRARIES
  )

  # Generate and install ConfigVersion.cmake
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipeline
  )

  # Generate and install Config.cmake
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

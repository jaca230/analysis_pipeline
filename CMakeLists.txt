cmake_minimum_required(VERSION 3.14)

project(analysis_pipeline VERSION 0.1 LANGUAGES CXX)

# Set the project namespace; overrideable by parent scope if desired
if(NOT DEFINED PROJECT_NAMESPACE)
  set(PROJECT_NAMESPACE analysis_pipeline)
endif()

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

# System dependencies
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist)
include(${ROOT_USE_FILE})

find_package(TBB REQUIRED)
find_package(analysis_pipeline_core REQUIRED)

# Gather source files, excluding main.cpp
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(FILTER SRC_FILES EXCLUDE REGEX "main\\.cpp$")

set(MAIN_EXECUTABLE_SRC src/main.cpp)

# Create shared library with project name as target
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
add_library(${PROJECT_NAMESPACE}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
add_library(${PROJECT_NAMESPACE}::pipeline ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 0
)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME} PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist
  TBB::tbb
  analysis_pipeline::analysis_pipeline_core
  analysis_pipeline::spdlog_header_only
  analysis_pipeline::nlohmann_json_header_only
)

# Executable target
add_executable(${PROJECT_NAME}_exec ${MAIN_EXECUTABLE_SRC})
target_link_libraries(${PROJECT_NAME}_exec PRIVATE ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME}_exec PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

# Install/export logic if top-level project
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)

  install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAMESPACE}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

  export(PACKAGE ${PROJECT_NAME})
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

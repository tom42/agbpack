# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.31.5)
project(agbpack VERSION 0.0.1 LANGUAGES C CXX)
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_LIST_DIR}/cmake")


################################################################################
# Options
################################################################################

option(agbpack_BUILD_TESTING "Build agbpack tests." ${PROJECT_IS_TOP_LEVEL})
option(agbpack_BUILD_AGBPACKER "Build the agbpacker executable." ${PROJECT_IS_TOP_LEVEL})


################################################################################
# Set up testing
################################################################################

if(agbpack_BUILD_TESTING)
  enable_testing()
  include(VtgTesting)
  vtg_testing_setup_catch2(3.7.1)
endif()


################################################################################
# Build our own code
################################################################################

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(VtgEnableWarnings)

# Create global configuration header
configure_file(
  agbpack_config.hpp.in
  "${CMAKE_CURRENT_BINARY_DIR}/agbpack_config.hpp")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory(src)
if(agbpack_BUILD_TESTING)
  add_subdirectory(test)
endif()


################################################################################
# Add custom targets (reuse lint etc.)
################################################################################

if(PROJECT_IS_TOP_LEVEL)
  add_custom_target(
    reuse-lint
    COMMAND reuse lint
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endif()

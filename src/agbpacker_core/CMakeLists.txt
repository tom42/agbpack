# SPDX-FileCopyrightText: 2026 Thomas Mathys
# SPDX-License-Identifier: MIT

include(FetchContent)
FetchContent_Declare(
  argpppp
  GIT_REPOSITORY https://github.com/tom42/argpppp
  GIT_TAG        v0.4.0)
FetchContent_MakeAvailable(argpppp)

set(
  agbpacker_core_sources
  PUBLIC FILE_SET CXX_MODULES FILES
  agbpacker_core.cppm
  PRIVATE
  command_line.cpp)

# Production version of agbpacker_core
add_library(agbpacker_core)
target_sources(agbpacker_core ${agbpacker_core_sources})
target_compile_definitions(agbpacker_core PRIVATE AGBPACK_EXPORT_FOR_UNIT_TESTING=)
target_compile_features(agbpacker_core PUBLIC cxx_std_23)
target_link_libraries(agbpacker_core PRIVATE agbpack argpppp)
vtg_target_enable_warnings(agbpacker_core)

# Unit testing version of agbpacker_core
if(agbpack_BUILD_TESTING)
  add_library(agbpacker_core_unit_testing)
  target_sources(agbpacker_core_unit_testing ${agbpacker_core_sources})
  target_compile_definitions(agbpacker_core_unit_testing PRIVATE AGBPACK_EXPORT_FOR_UNIT_TESTING=export)
  target_compile_features(agbpacker_core_unit_testing PUBLIC cxx_std_23)
  target_link_libraries(agbpacker_core_unit_testing PRIVATE agbpack argpppp)
  vtg_target_enable_warnings(agbpacker_core_unit_testing)
endif()

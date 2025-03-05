# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

set(
  argpppp_sources
  PUBLIC FILE_SET CXX_MODULES FILES
  argpppp.cppm
  of.cppm
  option.cppm
  optional_string.cppm
  parser.cppm
  PRIVATE
  argpppp.cpp)

# Production version of argpppp
add_library(argpppp)
target_sources(argpppp ${argpppp_sources})
target_compile_definitions(argpppp PRIVATE ARGPPPP_EXPORT_FOR_UNIT_TESTING=)

# Unit testing version of argpppp
if(BUILD_TESTING)
  add_library(argpppp_unit_testing)
  target_sources(argpppp_unit_testing ${argpppp_sources})
  target_compile_definitions(argpppp_unit_testing PRIVATE ARGPPPP_EXPORT_FOR_UNIT_TESTING=export)
endif()

# Check for argp_parse
include(CheckSymbolExists)
check_symbol_exists(argp_parse "argp.h" ARGPPPP_HAVE_ARGP)

# Use argp-standalone if argp_parse is not around
if(NOT ARGPPPP_HAVE_ARGP)
  include(FetchContent)
  FetchContent_Declare(
    argp-standalone
    GIT_REPOSITORY https://github.com/tom42/argp-standalone
    GIT_TAG        v1.0.2)
  FetchContent_MakeAvailable(argp-standalone)
  target_link_libraries(argpppp PRIVATE argp-standalone)
  target_link_libraries(argpppp_unit_testing PRIVATE argp-standalone)
endif()

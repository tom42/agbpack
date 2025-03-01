# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

set(
  argpppp_sources
  PUBLIC FILE_SET CXX_MODULES FILES
  argpppp.cppm
  PRIVATE
  argpppp.cpp)

add_library(argpppp)
target_sources(argpppp ${argpppp_sources})

# Use argp-standalone if argp_parse is not around
include(CheckFunctionExists)
check_function_exists(argp_parse ARGPPPP_HAVE_ARGP)
if(NOT ARGPPPP_HAVE_ARGP)
  include(FetchContent)
  FetchContent_Declare(
    argp-standalone
    GIT_REPOSITORY https://github.com/tom42/argp-standalone
    GIT_TAG        v1.0.2)
  FetchContent_MakeAvailable(argp-standalone)
  target_link_libraries(argpppp PRIVATE argp-standalone)
endif()

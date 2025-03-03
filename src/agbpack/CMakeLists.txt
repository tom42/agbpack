# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

configure_file(
  agbpack_config.hpp.in
  "${CMAKE_CURRENT_BINARY_DIR}/agbpack_config.hpp")

set(
  agbpack_sources
  PUBLIC FILE_SET CXX_MODULES FILES
  agbpack.cppm
  common.cppm
  delta.cppm
  exceptions.cppm
  huffman.cppm
  lzss.cppm
  header.cppm
  rle.cppm
  version.cppm
  PRIVATE
  header.cpp)

# Production version of agbpack.
add_library(agbpack)
target_sources(agbpack ${agbpack_sources})
target_compile_definitions(agbpack PRIVATE AGBPACK_EXPORT_FOR_UNIT_TESTING=)
target_include_directories(agbpack PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

# Unit testing version of agbpack.
# This may export more symbols than the production version, so that internals of agbpack can be tested.
if(BUILD_TESTING)
  add_library(agbpack_unit_testing)
  target_sources(agbpack_unit_testing ${agbpack_sources})
  target_compile_definitions(agbpack_unit_testing PRIVATE AGBPACK_EXPORT_FOR_UNIT_TESTING=export)
  target_include_directories(agbpack_unit_testing PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
endif()

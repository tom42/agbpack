# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

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
  PRIVATE
  cprs_huff.cpp
  header.cpp)

# Production version of agbpack.
add_library(agbpack)
target_sources(agbpack ${agbpack_sources})
target_compile_definitions(agbpack PRIVATE AGBPACK_EXPORT_FOR_UNIT_TESTING=)

# Unit testing version of agbpack.
# This may export more symbols than the production version, so that internals of agbpack can be tested.
add_library(agbpack_unit_testing)
target_sources(agbpack_unit_testing ${agbpack_sources})
target_compile_definitions(agbpack_unit_testing PRIVATE AGBPACK_EXPORT_FOR_UNIT_TESTING=export)

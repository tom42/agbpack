# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

set(
  agbpack_sources
  PUBLIC FILE_SET CXX_MODULES FILES
  agbpack.cppm
  agbpack-common.cppm
  agbpack-delta.cppm
  agbpack-exceptions.cppm
  agbpack-huffman.cppm
  agbpack-lzss.cppm
  agbpack-header.cppm
  agbpack-rle.cppm
  PRIVATE
  agbpack-header.cpp)

add_library(agbpack)
target_sources(agbpack ${agbpack_sources})
target_compile_definitions(agbpack PRIVATE AGBPACK_EXPORT_FOR_TESTING=)

add_library(agbpack_testing)
target_sources(agbpack_testing ${agbpack_sources})
target_compile_definitions(agbpack_testing PRIVATE AGBPACK_EXPORT_FOR_TESTING=export)

# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

add_library(agbpack_test_tools_common)
target_sources(
  agbpack_test_tools_common
  PUBLIC FILE_SET CXX_MODULES FILES
  agbpack_test_tools_common.cppm
  PRIVATE
  agbpack_test_tools_common.cpp)

add_executable(lzss-analyze lzss-analyze.cpp)
target_link_libraries(lzss-analyze PRIVATE agbpack agbpack_test_tools_common)

add_executable(lzss-encode lzss-encode.cpp)
target_link_libraries(lzss-encode PRIVATE agbpack agbpack_test_tools_common)

# TODO: we don't want to install these by default, no? This is just debug stuff for our own purpose...
install(TARGETS lzss-analyze lzss-encode DESTINATION bin)

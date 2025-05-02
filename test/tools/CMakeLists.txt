# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

add_library(agbpack_test_tools_common)
target_sources(
  agbpack_test_tools_common
  PUBLIC FILE_SET CXX_MODULES FILES
  agbpack_test_tools_common.cppm
  PRIVATE
  agbpack_test_tools_common.cpp)
vtg_target_enable_warnings(agbpack_test_tools_common)

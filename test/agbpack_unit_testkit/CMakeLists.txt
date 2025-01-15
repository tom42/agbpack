# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

# TODO: also rename the module itself
# TODO: and what about the namespace? Rename that too?
add_library(agbpack_unit_testkit)
target_sources(
  agbpack_unit_testkit
  PUBLIC FILE_SET CXX_MODULES FILES
  agbpack_unit_testkit.cppm
  PRIVATE
  agbpack_unit_testkit.cpp)
target_link_libraries(
  agbpack_unit_testkit
  PRIVATE
  agbpack_unit_testing)

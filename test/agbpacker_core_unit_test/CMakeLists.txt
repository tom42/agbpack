# SPDX-FileCopyrightText: 2026 Thomas Mathys
# SPDX-License-Identifier: MIT

add_executable(
  agbpacker_core_unit_test
  command_line_test.cpp)
vtg_target_enable_warnings(agbpacker_core_unit_test)
target_link_libraries(agbpacker_core_unit_test PRIVATE agbpacker_core_unit_testing Catch2::Catch2WithMain)
catch_discover_tests(agbpacker_core_unit_test)
# TODO: upgrade VtgEnableWarnings and use test settings for all tests?

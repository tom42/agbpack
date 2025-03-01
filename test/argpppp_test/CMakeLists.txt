# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

add_executable(
  agbpppp_test
  program_name_test.cpp)
target_link_libraries(agbpppp_test PRIVATE agbpack Catch2::Catch2WithMain)
catch_discover_tests(agbpppp_test)

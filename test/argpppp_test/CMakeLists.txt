# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

add_executable(
  argpppp_unit_test
  of_test.cpp
  option_test.cpp
  optional_string_test.cpp
  parser_test.cpp
  pf_test.cpp)
target_link_libraries(argpppp_unit_test PRIVATE argpppp_unit_testing Catch2::Catch2WithMain)
catch_discover_tests(argpppp_unit_test)

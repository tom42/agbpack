# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

add_executable(
  argpppp_unit_test
  of_test.cpp
  option_test.cpp
  option_callback_result_test.cpp
  optional_string_test.cpp
  parser_test.cpp
  program_name_test.cpp)
target_link_libraries(argpppp_unit_test PRIVATE argpppp_unit_testing Catch2::Catch2WithMain)
catch_discover_tests(argpppp_unit_test)

# Test whether defining argp_program_version works.
# If it does, invoking the program with --version should succeed.
add_executable(argp_program_version_test argp_program_version_test.cpp)
target_link_libraries(argp_program_version_test PRIVATE argpppp)
add_test(NAME argp_program_version_test COMMAND argp_program_version_test --version)

# SPDX-FileCopyrightText: 2025 Thomas Mathys
# SPDX-License-Identifier: MIT

add_executable(
  agbpppp_unit_test
  program_name_test.cpp)
target_link_libraries(agbpppp_unit_test PRIVATE argpppp Catch2::Catch2WithMain)
catch_discover_tests(agbpppp_unit_test)

# Test whether defining argp_program_version works.
# If it does, invoking the program with --version should succeed.
add_executable(argp_program_version_test argp_program_version_test.cpp)
target_link_libraries(argp_program_version_test PRIVATE argpppp)
add_test(NAME argp_program_version_test COMMAND argp_program_version_test --version)

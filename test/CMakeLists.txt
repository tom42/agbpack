# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

set(AGBPACK_TEST_TESTDATA_DIRECTORY "${PROJECT_SOURCE_DIR}/test/testdata")
configure_file(
  agbpack_test_config.hpp.in
  "${PROJECT_BINARY_DIR}/test/agbpack_test_config.hpp")

add_subdirectory(agbpack_unit_testkit)
add_subdirectory(agbpack_unit_test)
add_subdirectory(agbpack_test)
add_subdirectory(tools)

if(agbpack_BUILD_AGBPACKER)
  add_subdirectory(agbpacker_core_unit_test)
endif()

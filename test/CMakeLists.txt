# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

add_subdirectory(agbpack_unit_testkit)
add_subdirectory(agbpack_unit_test)
add_subdirectory(agbpack_test)
add_subdirectory(tools)

if(agbpack_BUILD_AGBPACKER)
  add_subdirectory(agbpacker_core_unit_test)
endif()

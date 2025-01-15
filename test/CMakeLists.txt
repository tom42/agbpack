# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # TODO: g++ bugs when <memory> and <filesystem> are used in the same translation unit.
  #       We need to fix this eventually (upgrade g++, rewrite or reorganize code),
  #       but for the time being we disable the warning.
  add_compile_options(-Wno-attributes)
endif()

add_subdirectory(agbpack_unit_testkit)
add_subdirectory(agbpack_unit_test)
add_subdirectory(agbpack_test)

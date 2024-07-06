# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

add_executable(
  agbpack_test
  rle_decoder_test.cpp)

target_link_libraries(agbpack_test PRIVATE agbpack Catch2::Catch2WithMain)
catch_discover_tests(agbpack_test)

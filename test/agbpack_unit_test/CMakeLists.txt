# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

add_executable(
  agbpack_unit_test
  header_test.cpp
  huffman_encoder_tree_test.cpp
  tree_node_test.cpp)

target_link_libraries(agbpack_unit_test PRIVATE agbpack_unit_testing Catch2::Catch2WithMain)
catch_discover_tests(agbpack_unit_test)

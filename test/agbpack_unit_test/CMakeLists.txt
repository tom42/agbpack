# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

add_executable(agbpack_unit_test)
target_sources(
  agbpack_unit_test
  PUBLIC FILE_SET CXX_MODULES FILES
  agbpack_unit_test.cppm
  PRIVATE
  agbpack_unit_test.cpp
  bitstream_writer_test.cpp
  header_test.cpp
  huffman_decoder_tree_test.cpp
  huffman_encoder_tree_test.cpp
  huffman_tree_node_test.cpp
  huffman_tree_serializer_test.cpp
  lzss_bitstream_writer_test.cpp
  match_finder_test.cpp
  node_priority_queue_test.cpp)

target_link_libraries(agbpack_unit_test PRIVATE agbpack_unit_testing Catch2::Catch2WithMain)
catch_discover_tests(agbpack_unit_test)

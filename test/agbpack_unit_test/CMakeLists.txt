# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # TODO: g++ bugs when <memory> and <filesystem> are used in the same translation unit.
  #       We need to fix this eventually (upgrade g++, rewrite or reorganize code),
  #       but for the time being we disable the warning.
  add_compile_options(-Wno-attributes)
endif()

add_executable(agbpack_unit_test)
target_sources(
  agbpack_unit_test
  PUBLIC FILE_SET CXX_MODULES FILES
  utility.cppm
  PRIVATE
  bitstream_writer_test.cpp
  header_test.cpp
  huffman_encoder_tree_test.cpp
  huffman_tree_node_test.cpp
  huffman_tree_serializer_test.cpp
  node_queue_test.cpp
  utility.cpp)

target_link_libraries(agbpack_unit_test PRIVATE agbpack_unit_testing Catch2::Catch2WithMain)
catch_discover_tests(agbpack_unit_test)

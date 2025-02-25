# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

set(AGBPACK_TEST_TESTDATA_DIRECTORY "${PROJECT_SOURCE_DIR}/test/testdata")
configure_file(
  agbpack_test_config.hpp.in
  "${CMAKE_CURRENT_BINARY_DIR}/agbpack_test_config.hpp")

add_executable(
  agbpack_test
  delta_decoder_test.cpp
  delta_encoder_test.cpp
  huffman_decoder_test.cpp
  huffman_encoder_test.cpp
  lzss_decoder_test.cpp
  lzss_encoder_test.cpp
  rle_encoder_test.cpp
  rle_decoder_test.cpp
  testdata.cpp)

target_include_directories(agbpack_test PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(agbpack_test PRIVATE agbpack Catch2::Catch2WithMain)
catch_discover_tests(agbpack_test)

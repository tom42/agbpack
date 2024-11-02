# SPDX-FileCopyrightText: 2024 Thomas Mathys
# SPDX-License-Identifier: MIT

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # TODO: g++ bugs when <memory> and <filesystem> are used in the same translation unit.
  #       We need to fix this eventually (upgrade g++, rewrite or reorganize code),
  #       but for the time being we disable the warning.
  add_compile_options(-Wno-attributes)
endif()

set(AGBPACK_TEST_TESTDATA_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/testdata")
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
  rle_encoder_test.cpp
  rle_decoder_test.cpp
  testdata.cpp)

target_include_directories(agbpack_test PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(agbpack_test PRIVATE agbpack_testing Catch2::Catch2WithMain)
catch_discover_tests(agbpack_test)

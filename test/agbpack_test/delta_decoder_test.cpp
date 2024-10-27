// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <string>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

using string = std::string;

TEST_CASE("delta_decoder_test")
{
    agbpack::delta_decoder decoder;
    test_data_directory test_data_directory("delta");

    SECTION("Valid input")
    {
        const string filename = GENERATE(
            "delta.good.8.zero-length-file.txt",
            "delta.good.8.one-byte.txt",
            "delta.good.8.sine.bin",
            "delta.good.16.zero-length-file.txt",
            "delta.good.16.one-word.bin",
            "delta.good.16.sine.bin");
        const auto expected_data = test_data_directory.read_decoded_file(filename);
        const auto encoded_data = test_data_directory.read_encoded_file(filename);

        const auto decoded_data = decode_vector(decoder, encoded_data);

        CHECK(decoded_data == expected_data);
    }

    SECTION("Invalid input")
    {
        const auto filename = GENERATE(
            "delta.bad.eof-inside-header.txt",
            "delta.bad.invalid-compression-type-in-header.txt",
            "delta.bad.valid-but-unexpected-compression-type-in-header.txt",
            "delta.bad.invalid-compression-options-in-header.txt",
            "delta.bad.8.eof-inside-stream.bin",
            "delta.bad.8.missing-padding-at-end-of-data.txt",
            "delta.bad.16.eof-inside-stream.bin",
            "delta.bad.16.missing-padding-at-end-of-data.bin");
        const auto encoded_data = test_data_directory.read_encoded_file(filename);

        CHECK_THROWS_AS(decode_vector(decoder, encoded_data), agbpack::decode_exception);
    }
}

}

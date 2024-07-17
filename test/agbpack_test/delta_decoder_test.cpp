// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <string>
#include "testdata.hpp"

import agbpack;

using string = std::string;

TEST_CASE("delta_decoder_test")
{
    agbpack::delta_decoder decoder;

    SECTION("Valid input")
    {
        // TODO: have each of these tests also for 16 bits?
        string filename_part = GENERATE(
            "delta.good.8.zero-length-file.txt",
            "delta.good.8.one-byte.txt",
            "delta.good.8.sine.bin",
            "delta.good.16.zero-length-file.txt",
            "delta.good.16.one-word.bin");
        auto expected_data = agbpack_test::read_file(filename_part + ".decoded");

        auto decoded_data = agbpack_test::decode_file(decoder, filename_part + ".encoded");

        CHECK(decoded_data == expected_data);
    }

    SECTION("Invalid input")
    {
        // TODO: 8 bit: uncompressed size in header too big
        auto encoded_file = GENERATE(
            "delta.bad.8.eof-inside-header.txt.encoded",
            "delta.bad.wrong-compression-type-in-header.txt.encoded",
            "delta.bad.wrong-compression-options-in-header.txt.encoded");

        CHECK_THROWS_AS(agbpack_test::decode_file(decoder, encoded_file), agbpack::bad_encoded_data);
    }
}

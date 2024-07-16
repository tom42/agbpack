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
        // TODO: figure out a very simple test case (e.g. just one byte, or word) and implement that. Zero length output would be pretty simple too.
        // TODO: also have delta.16.good-1-word.txt
        string filename_part = GENERATE("delta.good.8.1-byte.txt");
        auto expected_data = agbpack_test::read_file(filename_part + ".decoded");

        auto decoded_data = agbpack_test::decode_file(decoder, filename_part + ".encoded");

        CHECK(decoded_data == expected_data);
    }

    SECTION("Invalid input")
    {
        // TODO: wrong compression options
        // ...
        auto encoded_file = GENERATE(
            "delta.bad.8.eof-inside-header.txt.encoded",
            "delta.bad.wrong-compression-type-in-header.txt.encoded");

        CHECK_THROWS_AS(agbpack_test::decode_file(decoder, encoded_file), agbpack::bad_encoded_data);
    }
}

// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

import agbpack;

#include <string>
#include "testdata.hpp"

using string = std::string;

TEST_CASE("delta_decoder_test")
{
    SECTION("Valid input")
    {
        // TODO: figure out a very simple test case (e.g. just one byte, or word) and implement that. Zero length output would be pretty simple too.
        // TODO: also have delta.16.good-1-word.txt
        string filename_part = GENERATE("delta.8.good-1-byte.txt");
        auto expected_data = agbpack_test::read_file(filename_part + ".decoded");
        agbpack::delta_decoder decoder; // TODO: where do we create this? does it even matter?

        auto decoded_data = agbpack_test::decode_file(decoder, filename_part + ".encoded");

        CHECK(decoded_data == expected_data);
    }

    SECTION("Invalid input")
    {
        // TODO: eof while reading header
        // TODO: wrong compression type
        // TODO: wrong compression options
        // ...
    }
}

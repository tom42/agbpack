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

TEST_CASE("delta_encoder_test")
{
    agbpack::delta_encoder encoder;

    SECTION("8 bit")
    {
        // TODO: many bytes of input

        // TODO: sketch out how encoding works
        //       * set options
        //       * call encode
        // TODO: verify using the decoder (orly?)
        //       * The decoder must be able to decode
        //       * The decoded data must be the same as the original data

        encoder.options(agbpack::delta_options::delta8);

        string filename_part = GENERATE(
            "delta.good.8.zero-length-file.txt",
            "delta.good.8.one-byte.txt",
            "delta.good.8.sine.bin");
        auto expected_data = read_file(filename_part + ".encoded");

        auto encoded_data = encode_file(encoder, filename_part + ".decoded");

        CHECK(encoded_data == expected_data);
    }

    SECTION("16 bit")
    {
        // TODO: one byte of input (orly? should it not be a word?)
        // TODO: many bytes of input
        // TODO: what if input is an odd number of bytes?

        encoder.options(agbpack::delta_options::delta16);

        string filename_part = GENERATE(
            "delta.good.16.zero-length-file.txt");
        auto expected_data = read_file(filename_part + ".encoded");

        auto encoded_data = encode_file(encoder, filename_part + ".decoded");

        CHECK(encoded_data == expected_data);
    }
}

}

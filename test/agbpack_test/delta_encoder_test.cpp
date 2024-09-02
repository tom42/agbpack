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

        // TODO: verify using the decoder (orly?)
        //       * The decoder must be able to decode
        //       * The decoded data must be the same as the original data
        string filename_part = GENERATE(
            "delta.good.8.zero-length-file.txt",
            "delta.good.8.one-byte.txt",
            "delta.good.8.sine.bin");
        auto expected_data = read_file(filename_part + ".encoded");

        encoder.options(agbpack::delta_options::delta8);
        auto encoded_data = encode_file(encoder, filename_part + ".decoded");

        CHECK(encoded_data == expected_data);
    }

    SECTION("16 bit")
    {
        // TODO: many bytes of input
        // TODO: what if input is an odd number of bytes? => Well that's an error, innit? What do other encoders do in that case?
        string filename_part = GENERATE(
            "delta.good.16.zero-length-file.txt",
            "delta.good.16.one-word.bin");
        auto expected_data = read_file(filename_part + ".encoded");

        encoder.options(agbpack::delta_options::delta16);
        auto encoded_data = encode_file(encoder, filename_part + ".decoded");

        CHECK(encoded_data == expected_data);
    }
}

}

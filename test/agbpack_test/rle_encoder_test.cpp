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

TEST_CASE("rle_encoder_test")
{
    agbpack::rle_encoder encoder;

    SECTION("Successful encoding")
    {
        // TODO: clean up rle test file names
        //       * "compressed [byte(s)]" => "repeated [byte(s)]"
        //       * uncompressed => literal
        // TODO: see what else we want to test (same as for the decoder? otoh it is also given by the test first development of the encoder)
        string filename_part = GENERATE(
            "rle.good.zero-length-file.txt"
            // TODO: 1 literal
            // TODO: 2 literals
            // TODO: multiple of max literals and then some, so that buffer flush is required inside loop
            );
        auto expected_data = read_file(filename_part + ".encoded");

        auto encoded_data = encode_file(encoder, filename_part + ".decoded");

        CHECK(encoded_data == expected_data);
    }
}

}

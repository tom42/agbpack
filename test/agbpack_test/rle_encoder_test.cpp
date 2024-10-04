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
        // TODO: clean up rle test file names (also those from the decoder test!)
        //       * "compressed [byte(s)]" => "repeated [byte(s)]"
        //       * uncompressed => literal
        // TODO: see what else we want to test (same as for the decoder? otoh it is also given by the test first development of the encoder)
        string filename_part = GENERATE(
            "rle.good.zero-length-file.txt",
            "rle.good.1-literal-byte.txt",
            "rle.good.2-literal-bytes.txt",
            "rle.good.131-literal-bytes.txt",
            // TODO: also test special case of 2 repeated bytes followed by something else (?)
            // TODO: also test very special case of 2 repeated bytes where adding them to the literal buffer overflows the buffer (max literal run length)
            "rle.good.2-repeated-bytes.txt",
            "rle.good.3-repeated-bytes.txt",
            "rle.good.4-repeated-bytes.txt",
            "rle.good.131-repeated-bytes.txt",
            "rle.good.literal-buffer-overflow-by-2-repeated-bytes.txt");
        auto expected_data = read_file(filename_part + ".encoded");

        auto encoded_data = encode_file(encoder, filename_part + ".decoded");

        CHECK(encoded_data == expected_data);
    }
}

}

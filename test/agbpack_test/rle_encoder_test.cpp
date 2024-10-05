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
        string filename_part = GENERATE(
            // Test data designed for encoder development
            "rle.good.zero-length-file.txt",
            "rle.good.1-literal-byte.txt",
            "rle.good.2-literal-bytes.txt",
            "rle.good.131-literal-bytes.txt",
            "rle.good.2-repeated-bytes.txt",
            "rle.good.3-repeated-bytes.txt",
            "rle.good.4-repeated-bytes.txt",
            "rle.good.131-repeated-bytes.txt",
            "rle.good.literal-buffer-overflow-by-2-repeated-bytes.txt",
            "rle.good.literal-bytes-followed-by-repeated-bytes.txt",
            // Test data from decoder development, added here for good measure
            "rle.good.3-literal-bytes.txt",
            "rle.good.5-literal-bytes.txt",
            "rle.good.foo.txt",
            "rle.good.very-long-literal-run.txt",
            "rle.good.very-long-repeated-run.txt");
        auto expected_data = read_file(filename_part + ".encoded");

        auto encoded_data = encode_file(encoder, filename_part + ".decoded");

        CHECK(encoded_data == expected_data);
    }
}

}

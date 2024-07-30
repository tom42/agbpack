// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

TEST_CASE("huffman_decoder_test")
{
    agbpack::huffman_decoder decoder;
    (void)decoder; // TODO: remove

    SECTION("Invalid input")
    {
        // TODO: invalid compression options
        auto encoded_file = GENERATE(
            "huffman.bad.eof-inside-header.txt.encoded",
            "huffman.bad.invalid-compression-type-in-header.txt.encoded",
            "huffman.bad.valid-but-unexpected-compression-type-in-header.txt.encoded");

        CHECK_THROWS_AS(decode_file(decoder, encoded_file), agbpack::bad_encoded_data);
    }
}

}

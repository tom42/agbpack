// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "testdata.hpp"

import agbpack;

TEST_CASE("lzss_decoder_test")
{
    agbpack::lzss_decoder decoder;
    (void)decoder; // TODO: remove

    SECTION("Invalid input")
    {
        auto encoded_file = GENERATE(
            "lzss.bad.eof-inside-header.txt.encoded",
            // TODO: wrong compression type: we must now distinguish between
            //       * wrong compression type (that is, a valid but wrong one)
            //       * completely invalid compression type
            //       * Once we drop in new header parsing code into other decoders we must do so there too.
            //         * That is, once we've settled for an API, change other tests too!
            "lzss.bad.wrong-compression-type-in-header.txt.encoded"); // TODO: fix this file: use a valid-but-wrong compression type

        CHECK_THROWS_AS(agbpack_test::decode_file(decoder, encoded_file), agbpack::bad_encoded_data);
    }
}

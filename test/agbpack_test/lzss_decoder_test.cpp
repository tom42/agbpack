// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "testdata.hpp"

import agbpack;

using string = std::string;

TEST_CASE("lzss_decoder_test")
{
    agbpack::lzss_decoder decoder;

    SECTION("Valid input")
    {
        // TODO: short simple file without compression (e.g. 1 byte)
        // TODO: not so short file without compression
        // TODO: files with compression
        //       * shortest offset
        //       * longest offset
        //       * shortest length
        //       * longest length
        string filename_part = GENERATE(
            "lzss.good.uncompressed-bytes-only-1.txt");
        auto expected_data = agbpack_test::read_file(filename_part + ".decoded");

        auto decoded_data = agbpack_test::decode_file(decoder, filename_part + ".encoded");

        CHECK(decoded_data == expected_data);
    }

    SECTION("Invalid input")
    {
        auto encoded_file = GENERATE(
            "lzss.bad.eof-inside-header.txt.encoded",
            // TODO: wrong compression type: we must now distinguish between
            //       * wrong compression type (that is, a valid but wrong one)
            //       * completely invalid compression type
            //       * Once we drop in new header parsing code into other decoders we must do so there too.
            //         * That is, once we've settled for an API, change other tests too!
            "lzss.bad.invalid-compression-type-in-header.txt.encoded",                 // TODO: ensure this is an otherwise valid zero length file
            "lzss.bad.valid-but-unexpected-compression-type-in-header.txt.encoded",    // TODO: ensure this is an otherwise valid zero length file
            "lzss.bad.invalid-compression-options-in-header.txt.encoded");             // TODO: ensure this is an otherwise valid zero length file

        CHECK_THROWS_AS(agbpack_test::decode_file(decoder, encoded_file), agbpack::bad_encoded_data);
    }
}

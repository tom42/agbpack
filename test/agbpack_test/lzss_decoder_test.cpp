// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <filesystem> // TODO: remove
#include "testdata.hpp"

import agbpack;

using string = std::string;

namespace agbpack_test
{

namespace
{

// TODO: review thoroughly
template <typename TDecoder>
std::vector<unsigned char> decode_file_to_vector(TDecoder& decoder, const std::string& basename)
{
    // TODO: hack: read file to determine its size. Use getfilesize method. We have something like that
    // TODO: hack: also ugly: the fact that we need to replace extensions here again
    auto uncompressed_size = read_file(std::filesystem::path(basename).replace_extension("decoded").string()).size();

    std::vector<unsigned char> input = read_file(basename);
    std::vector<unsigned char> output(uncompressed_size);
    decoder.decode(input.begin(), input.end(), begin(output));
    return output;
}

}

TEST_CASE("lzss_decoder_test")
{
    agbpack::lzss_decoder decoder;

    SECTION("Valid input")
    {
        // TODO: not so short file without compression (e.g. 8 bytes)
        //       * And an even longer file > 8 bytes => That would require us to start reading flag bytes
        // TODO: files with compression
        //       * shortest offset
        //       * longest offset
        //       * shortest length
        //       * longest length
        string filename_part = GENERATE(
            "lzss.good.1-uncompressed-byte.txt",
            "lzss.good.8-uncompressed-bytes.txt",
            "lzss.good.17-uncompressed-bytes.txt",
            "lzss.good.compressed-bytes-1.txt",
            "lzss.good.compressed-bytes-2.txt",
            "foo.txt"); // TODO: rename this: this is a file with a maximum offset
        auto expected_data = read_file(filename_part + ".decoded");

        CHECK(decode_file(decoder, filename_part + ".encoded") == expected_data);
        CHECK(decode_file_to_vector(decoder, filename_part + ".encoded") == expected_data);
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

        CHECK_THROWS_AS(decode_file(decoder, encoded_file), agbpack::bad_encoded_data);
        CHECK_THROWS_AS(decode_file_to_vector(decoder, encoded_file), agbpack::bad_encoded_data);
    }
}

}

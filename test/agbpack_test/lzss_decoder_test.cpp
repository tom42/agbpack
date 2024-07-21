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
std::vector<unsigned char> decode_file_to_vector(TDecoder& decoder, const std::string& basename, bool expect_successful_decoding)
{
    // TODO: hack: read file to determine its size. Use getfilesize method. We have something like that
    // TODO: hack: also ugly: the fact that we need to replace extensions here again
    // TODO: the expect_successful_decoding thing is also a hack. But it gets us going for the moment.
    //       What we really want is go get at the ader, so that we can do header parsing ourselves.
    auto uncompressed_size = expect_successful_decoding
        ? read_file(std::filesystem::path(basename).replace_extension("decoded").string()).size()
        : 16384;

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
        // TODO: files with compression
        //       * shortest offset
        //       * longest offset
        //       * shortest length
        //       * longest length
        // TODO: file with and without compressed runs
        string filename_part = GENERATE(
            "lzss.good.1-uncompressed-byte.txt",
            "lzss.good.8-uncompressed-bytes.txt",
            "lzss.good.17-uncompressed-bytes.txt",
            "lzss.good.compressed-bytes-1.txt",
            "lzss.good.compressed-bytes-2.txt",
            "lzss.good.zero-length-file.txt",
            "foo.txt"); // TODO: rename this: this is a file with a maximum offset
        auto expected_data = read_file(filename_part + ".decoded");

        CHECK(decode_file(decoder, filename_part + ".encoded") == expected_data);
        CHECK(decode_file_to_vector(decoder, filename_part + ".encoded", true) == expected_data);
    }

    SECTION("Invalid input")
    {
        auto encoded_file = GENERATE(
            "lzss.bad.eof-inside-header.txt.encoded",
            "lzss.bad.invalid-compression-type-in-header.txt.encoded",
            "lzss.bad.valid-but-unexpected-compression-type-in-header.txt.encoded",
            "lzss.bad.invalid-compression-options-in-header.txt.encoded");

        CHECK_THROWS_AS(decode_file(decoder, encoded_file), agbpack::bad_encoded_data);
        CHECK_THROWS_AS(decode_file_to_vector(decoder, encoded_file, false), agbpack::bad_encoded_data);
    }
}

}

// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <iterator>
#include <string>
#include <vector>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

using string = std::string;
template <typename T> using vector = std::vector<T>;

TEST_CASE("rle_decoder_test")
{
    agbpack::rle_decoder decoder;

    SECTION("Valid input")
    {
        string filename_part = GENERATE(
            "rle.good.3-literal-bytes.txt",
            "rle.good.5-literal-bytes.txt",
            "rle.good.3-repeated-bytes.txt",
            "rle.good.literal-bytes-followed-by-repeated-bytes.txt",
            "rle.good.very-long-compressed-run.txt",
            "rle.good.very-long-uncompressed-run.txt",
            "rle.good.zero-length-file.txt",
            "rle.good.foo.txt");
        auto expected_data = read_file(filename_part + ".decoded");

        auto decoded_data = decode_file(decoder, filename_part + ".encoded");

        CHECK(decoded_data == expected_data);
    }

    SECTION("Invalid input")
    {
        auto encoded_file = GENERATE(
            "rle.bad.eof-inside-header.txt.encoded",
            "rle.bad.eof-at-flag-byte.txt.encoded",
            "rle.bad.eof-at-compressed-byte.txt.encoded",
            "rle.bad.eof-inside-uncompressed-run.txt.encoded",
            "rle.bad.compressed-run-goes-past-decompressed-size.txt.encoded",
            "rle.bad.uncompressed-run-goes-past-decompressed-size.txt.encoded",
            "rle.bad.invalid-compression-type-in-header.txt.encoded",
            "rle.bad.valid-but-unexpected-compression-type-in-header.txt.encoded",
            "rle.bad.invalid-compression-options-in-header.txt.encoded",
            "rle.bad.missing-padding-at-end-of-data.txt.encoded");

        CHECK_THROWS_AS(decode_file(decoder, encoded_file), agbpack::decode_exception);
    }

    SECTION("Input from ifstream")
    {
        auto path = get_testfile_path("rle.good.foo.txt.encoded");
        auto file = open_binary_file(path);
        vector<unsigned char> decoded_data;

        decoder.decode(
            std::istream_iterator<unsigned char>(file),
            std::istream_iterator<unsigned char>(),
            back_inserter(decoded_data));

        CHECK(decoded_data == read_file("rle.good.foo.txt.decoded"));
    }
}

}

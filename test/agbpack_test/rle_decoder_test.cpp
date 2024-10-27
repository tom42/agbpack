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
    test_data_directory test_data_directory("rle");

    SECTION("Valid input")
    {
        const string filename = GENERATE(
            "rle.good.3-literal-bytes.txt",
            "rle.good.5-literal-bytes.txt",
            "rle.good.3-repeated-bytes.txt",
            "rle.good.literal-bytes-followed-by-repeated-bytes.txt",
            "rle.good.very-long-literal-run.txt",
            "rle.good.very-long-repeated-run.txt",
            "rle.good.zero-length-file.txt",
            "rle.good.foo.txt");
        const auto expected_data = test_data_directory.read_decoded_file(filename);
        const auto encoded_data = test_data_directory.read_encoded_file(filename);

        const auto decoded_data = decode_vector(decoder, encoded_data);

        CHECK(decoded_data == expected_data);
    }

    SECTION("Invalid input")
    {
        const auto filename = GENERATE(
            "rle.bad.eof-inside-header.txt",
            "rle.bad.eof-at-flag-byte.txt",
            "rle.bad.eof-at-repeated-byte.txt",
            "rle.bad.eof-inside-literal-run.txt",
            "rle.bad.literal-run-goes-past-decompressed-size.txt",
            "rle.bad.repeated-run-goes-past-decompressed-size.txt",
            "rle.bad.invalid-compression-type-in-header.txt",
            "rle.bad.valid-but-unexpected-compression-type-in-header.txt",
            "rle.bad.invalid-compression-options-in-header.txt",
            "rle.bad.missing-padding-at-end-of-data.txt");
        const auto encoded_data = test_data_directory.read_encoded_file(filename);

        CHECK_THROWS_AS(decode_vector(decoder, encoded_data), agbpack::decode_exception);
    }

    SECTION("Input from ifstream")
    {
        const auto path = test_data_directory.get_testfile_path("rle.good.foo.txt.encoded");
        auto file = open_binary_file(path);
        vector<unsigned char> decoded_data;

        decoder.decode(
            std::istream_iterator<unsigned char>(file),
            std::istream_iterator<unsigned char>(),
            back_inserter(decoded_data));

        CHECK(decoded_data == test_data_directory.read_decoded_file("rle.good.foo.txt"));
    }
}

}

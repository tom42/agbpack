// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <iterator>
#include <string>
#include <vector>
#include "testdata.hpp"

import agbpack;

using string = std::string;

namespace
{
    
std::vector<unsigned char> decode_file(const string& basename)
{
    const auto name = agbpack_test::get_testfile_path(basename);

    auto file = agbpack_test::open_binary_file(name);
    agbpack::rle_decoder decoder;

    std::vector<unsigned char> output;
    decoder.decode(
        std::istream_iterator<unsigned char>(file),
        std::istream_iterator<unsigned char>(),
        std::back_inserter(output));
    return output;
}

}

TEST_CASE("rle_decoder")
{
    // TODO: pathological stuff: malformed streams, too long, too short, malformed header
    // TODO: also test using a container as input? E.g. read from vector<unsigned char>?

    // TODO: Make the decoder work with different element types. E.g. have it be able to process char, unsigned char, byte, whatever.
    //       Question is then, do we allow input element type to be different from output element type?
    // TODO: should probably also decode from vector: this might have better debug facility than a stream

    SECTION("Valid input")
    {
        string filename_part = GENERATE(
            "rle.good.uncompressed-bytes-only-1.txt",
            "rle.good.uncompressed-bytes-only-2.txt",
            "rle.good.compressed-bytes-only.txt",
            "rle.good.compressed-and-uncompressed-bytes.txt",
            "rle.good.very-long-compressed-run.txt",
            "rle.good.very-long-uncompressed-run.txt",
            "rle.good.zero-length-file.txt",
            "rle.good.foo.txt");
        auto expected_data = agbpack_test::read_testfile(filename_part + ".decoded");

        auto decoded_data = decode_file(filename_part + ".encoded");

        CHECK(decoded_data == expected_data);
    }

    SECTION("Premature end of input")
    {
        auto encoded_file = GENERATE(
            "rle.bad.eof-inside-header.txt.encoded",
            "rle.bad.eof-at-flag-byte.txt.encoded",
            "rle.bad.eof-at-compressed-byte.txt.encoded",
            "rle.bad.eof-inside-uncompressed-run.txt.encoded",
            "rle.bad.compressed-run-goes-past-decompressed-size.txt.encoded",
            "rle.bad.uncompressed-run-goes-past-decompressed-size.txt.encoded");

        CHECK_THROWS_AS(decode_file(encoded_file), agbpack::bad_encoded_data);
    }
}

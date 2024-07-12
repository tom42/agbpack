// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <iterator>
#include <vector>
#include "testdata.hpp"

import agbpack;

namespace
{
    std::vector<unsigned char> decode_testfile(const char* basename)
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
    // TODO: decode a file with compression only
    // TODO: decode a file with literals and runs
    // TODO: decode a zero length file
    // TODO: pathological stuff: malformed streams, too long, too short, malformed header
    // TODO: also test using a container as input? E.g. read from vector<unsigned char>?
    // TODO: test a file with a longer than maximum run of compressed byte
    // TODO: test a file with a longer than maximum run of literal bytes

    // TODO: assert something: return value (hardcode, can be 'abc'
    //       Problem is much more: how do we specify element type? unsigned char, or std::byte? Are we even going to make it using std::byte? Should we attempt to do so?
    // TODO: no but maybe test with different types of inputs? We're already doing this, after all...
    //       => E.g. read std::byte but output unsigned char
    // TODO: should probably also decode from vector: this might have better debug facility than a stream
    // TODO: naming of files: what's a literal? what's a run? Maybe use GBATEK terminology (compressed/uncompressed byte)

    SECTION("Valid input")
    {
        CHECK(decode_testfile("rle.literals-only.txt.compressed") == agbpack_test::read_testfile("rle.literals-only.txt.uncompressed"));
        CHECK(decode_testfile("rle.literals-only-2.txt.compressed") == agbpack_test::read_testfile("rle.literals-only-2.txt.uncompressed"));
        CHECK(decode_testfile("rle.runs-only-1.txt.compressed") == agbpack_test::read_testfile("rle.runs-only-1.txt.uncompressed"));
        CHECK(decode_testfile("rle.literals-and-runs-1.txt.compressed") == agbpack_test::read_testfile("rle.literals-and-runs-1.txt.uncompressed"));
    }

    SECTION("EOF inside header")
    {
        // TODO: add test cases for all places where we read input so far
        CHECK_THROWS_AS(decode_testfile("rle.broken.eof-inside-header.txt.compressed"), agbpack::bad_compressed_data);
    }
}

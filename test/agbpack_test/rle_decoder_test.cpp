// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

import agbpack;

#include <catch2/catch_test_macros.hpp>
#include <cerrno>
#include <fstream>
#include <iterator>
#include <vector>
#include "testdata.hpp"

namespace
{
    std::vector<unsigned char> decode_testfile(const char* basename)
    {
        const auto name = agbpack_test::get_testfile_path(basename);

        std::ifstream f;

        // TODO: do we need to disable whitespace skipping here? => Yes, definitely, especially now that we use istream_iterator!
        // TODO: letting streams throw exceptions is somewhat silly: if the file cannot be opened we tend to get rather useless error messages ("failbit set", "stream error", that sort)
        // TODO: definitely should handle errors ourselves here and only later set exceptions: exception messages suck, srsly.
        f.open(name, std::ios_base::binary);
        f.unsetf(std::ios::skipws);

        agbpack::rle_decoder decoder;

        std::vector<unsigned char> output;
        decoder.decode(
            std::istream_iterator<unsigned char>(f),
            std::istream_iterator<unsigned char>(),
            std::back_inserter(output));
        return output;
    }

}

TEST_CASE("rle_decoder")
{
    // TODO: decode a file with compression
    // TODO: decode a zero length file
    // TODO: pathological stuff: malformed streams, too long, too short, malformed header
    // TODO: also test using a container as input? E.g. read from vector<unsigned char>?

    // TODO: assert something: return value (hardcode, can be 'abc'
    //       Problem is much more: how do we specify element type? unsigned char, or std::byte? Are we even going to make it using std::byte? Should we attempt to do so?
    // TODO: no but maybe test with different types of inputs? We're already doing this, after all...
    //       => E.g. read std::byte but output unsigned char
    // TODO: should probably also decode from vector: this might have better debug facility than a stream
    CHECK(decode_testfile("rle.literals-only.txt.compressed") == agbpack_test::read_testfile("rle.literals-only.txt.uncompressed"));
    CHECK(decode_testfile("rle.literals-only-2.txt.compressed") == agbpack_test::read_testfile("rle.literals-only-2.txt.uncompressed"));
}

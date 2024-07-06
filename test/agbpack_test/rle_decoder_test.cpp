// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

import agbpack;

#include <catch2/catch_test_macros.hpp>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>
#include "agbpack_test_config.hpp"
#include "testdata.hpp"

namespace
{
    std::vector<unsigned char> decode_testdata_file(const char* basename)
    {
        const auto name = std::filesystem::path(agbpack_test_testdata_directory) / basename;

        std::ifstream f;
        f.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit); // TODO: do we want eofbit too?

        // TODO: do we need to disable whitespace skipping here?
        // TODO: letting streams throw exceptions is somewhat silly: if the file cannot be opened we tend to get rather useless error messages ("failbit set", "stream error", that sort)
        // TODO: definitely should handle errors ourselves here and only later set exceptions: exception messages suck, srsly.
        // TODO: use utility file to get path to test data directory (and put agbpack_test_testdata_directory into a namespace?)
        f.open(name, std::ios_base::binary);

        std::istreambuf_iterator<char> input(f.rdbuf());
        std::istreambuf_iterator<char> eof;

        agbpack::rle_decoder decoder;

        std::vector<unsigned char> output;
        decoder.decode(input, eof, std::back_inserter(output));
        return output;
    }

}

TEST_CASE("rle_decoder")
{
    // TODO: decode another simple file: no compression, but different size => this will mean that we have to read the depacked size from the header
    // TODO: decode a file with compression
    // TODO: decode a zero length file
    // TODO: pathological stuff: malformed streams, too long, too short, malformed header

    // TODO: assert something: return value (hardcode, can be 'abc'
    //       Problem is much more: how do we specify element type? unsigned char, or std::byte? Are we even going to make it using std::byte? Should we attempt to do so?
    // TODO: no but maybe test with different types of inputs? We're already doing this, after all...
    //       => E.g. read std::byte but output unsigned char
    REQUIRE(decode_testdata_file("rle.literals-only.txt.compressed") == agbpack_test::read_testdata_file("rle.literals-only.txt.uncompressed"));
    REQUIRE(decode_testdata_file("rle.literals-only-2.txt.compressed") == agbpack_test::read_testdata_file("rle.literals-only-2.txt.uncompressed"));
}

// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

import agbpack;

#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <iterator>
#include <vector>

namespace
{
    // TODO: pass filename from test
    std::vector<unsigned char> decode_file()
    {
        std::ifstream f;
        f.exceptions(std::ifstream::badbit | std::ifstream::failbit); // TODO: do we want eofbit too?

        // TODO: unhardcode path, one way or another.
        // TODO: letting streams throw exceptions is somewhat silly: if the file cannot be opened we tend to get rather useless error messages ("failbit set", "stream error", that sort)
        f.open("C:\\Users\\mathy\\Desktop\\work\\github\\agbpack\\test\\agbpack_test\\data\\rle.literals-only.txt.compressed", std::ios_base::binary);

        // TODO: somehow, pass in data to decode (e.g. a file. Maybe name it decode_file, or rle_decode_file)
        // TODO: open file, pass stream stuff to decoder for decoding
        // TODO: decoder should take a back_inserter or something
        agbpack::rle_decoder decoder;
        // TODO: pass real start to input (what we're doing below is not good and is only there to satisfy the compiler)
        // TODO: pass real end of input

        std::vector<unsigned char> output;
        decoder.decode(output.begin(), output.end(), std::back_inserter(output));
        return output;
    }
}

TEST_CASE("rle_decoder")
{
    // TODO: decode a first simple file: maybe one without any compression at all
    // TODO: decode a file with compression
    // TODO: decode a zero length file
    // TODO: pathological stuff: malformed streams, too long, too short, malformed header
    // TODO: need to figure out a way how to make data files available to tests:
    //       * Bake path into test suite
    //       * Copy data files to output directory


    // TODO: assert something: return value (hardcode, can be 'abc'
    //       Problem is much more: how do we specify element type? unsigned char, or std::byte? Are we even going to make it using std::byte? Should we attempt to do so?
    REQUIRE(decode_file() == std::vector<unsigned char>{'a', 'b', 'c'});
}
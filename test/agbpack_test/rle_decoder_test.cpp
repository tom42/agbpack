// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

import agbpack;

#include <catch2/catch_test_macros.hpp>
#include <iterator>
#include <vector>

namespace
{
    std::vector<unsigned char> decode()
    {
        // TODO: somehow, pass in data to decode (e.g. a file. Maybe name it decode_file, or rle_decode_file)
        // TODO: open file, pass stream stuff to decoder for decoding
        // TODO: decoder should take a back_inserter or something
        agbpack::rle_decoder decoder;
        // TODO: pass start to input
        // TODO: pass end of input

        std::vector<unsigned char> output;
        decoder.decode(std::back_inserter(output));
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
    REQUIRE(decode() == std::vector<unsigned char>{'a', 'b', 'c'});
}
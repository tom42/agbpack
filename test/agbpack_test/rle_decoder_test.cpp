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
    std::vector<unsigned char> decode_file(const char* /*name*/)
    {
        std::ifstream f;
        f.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit); // TODO: do we want eofbit too?

        // TODO: unhardcode path, one way or another.
        // TODO: letting streams throw exceptions is somewhat silly: if the file cannot be opened we tend to get rather useless error messages ("failbit set", "stream error", that sort)
        f.open("C:\\Users\\mathy\\Desktop\\work\\github\\agbpack\\test\\agbpack_test\\data\\rle.literals-only.txt.compressed", std::ios_base::binary);

        std::istreambuf_iterator<char> input(f.rdbuf());
        std::istreambuf_iterator<char> eof;

        // TODO: somehow, pass in data to decode (e.g. a file. Maybe name it decode_file, or rle_decode_file)
        // TODO: open file, pass stream stuff to decoder for decoding
        // TODO: decoder should take a back_inserter or something
        agbpack::rle_decoder decoder;

        std::vector<unsigned char> output;
        decoder.decode(input, eof, std::back_inserter(output));
        return output;
    }

    std::vector<unsigned char> read_file(const char* /*name*/)
    {
        // TODO: open file, throw exception if failed (orly?)
        // TODO: what ios flags do we need? which ones we don't?
        // TODO: do we need to disable white space skipping?
        // TODO: get size of file. How? seek/tell hack? Filesystem library
        // TODO: create vector, somehow read data

        return std::vector<unsigned char>();
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
    // TODO: no but maybe test with different types of inputs? We're already doing this, after all...
    // TODO: pass in input file name from here
    // TODO: unhardcode expected uncompressed data
    REQUIRE(decode_file("rle.literals-only.txt.compressed") == read_file("rle.literals-only.txt.uncompressed"));
}
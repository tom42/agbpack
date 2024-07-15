// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

import agbpack;

// TODO: once decode_file is shared, this should not be needed anymore
#include <vector>
#include "testdata.hpp"
using string = std::string;
template <typename T> using vector = std::vector<T>;

namespace
{

// TODO: share this with rle_decoder_test.cpp somehow
//       => Can we not make the decoder somehow a template argument?
vector<unsigned char> decode_file(const string& basename)
{
    vector<unsigned char> input = agbpack_test::read_file(basename);
    vector<unsigned char> output;
    agbpack::delta_decoder decoder;
    decoder.decode(input.begin(), input.end(), back_inserter(output));
    return output;
}

}

TEST_CASE("delta_decoder_test")
{
    SECTION("Valid input")
    {
        // TODO: figure out a very simple test case (e.g. just one byte, or word) and implement that. Zero length output would be pretty simple too.
        // TODO: also have delta.16.good-1-word.txt
        string filename_part = GENERATE("delta.8.good-1-byte.txt");
        auto expected_data = agbpack_test::read_file(filename_part + ".decoded");

        auto decoded_data = decode_file(filename_part + ".encoded");

        CHECK(decoded_data == expected_data);
    }

    SECTION("Invalid input")
    {
        // TODO: eof while reading header
        // TODO: wrong compression type
        // TODO: wrong compression options
        // ...
    }
}

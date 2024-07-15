// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

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
    }

    SECTION("Invalid input")
    {
    }
}

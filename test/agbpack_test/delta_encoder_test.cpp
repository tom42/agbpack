// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <vector>

import agbpack;

namespace agbpack_test
{

TEST_CASE("delta_encoder_test")
{
    SECTION("8 bit")
    {
        // TODO: zero bytes of input
        // TODO: one byte of input
        // TODO: many bytes of input

        // TODO: sketch out how encoding works
        //       * set options
        //       * call encode
        // TODO: verify using the decoder
        //       * The decoder must be able to decode
        //       * The decoded data must be the same as the original data

        std::vector<unsigned char> input;
        std::vector<unsigned char> output;

        agbpack::delta_encoder encoder;
        encoder.encode(input.begin(), input.end(), back_inserter(output));
    }

    SECTION("16 bit")
    {
        // TODO: zero bytes of input
        // TODO: one byte of input
        // TODO: many bytes of input
    }
}

}

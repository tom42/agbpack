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

        std::vector<unsigned char> encoded_data;
        std::vector<unsigned char> decoded_data;

        agbpack::delta_encoder encoder;
        encoder.encode(encoded_data.begin(), encoded_data.end(), back_inserter(decoded_data));

        // TODO: real expected data (easy: header + zero bytes of output)
        CHECK(decoded_data == std::vector<unsigned char>{ 0x81, 0, 0, 0 });
    }

    SECTION("16 bit")
    {
        // TODO: zero bytes of input
        // TODO: one byte of input
        // TODO: many bytes of input
    }
}

}

// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

TEST_CASE_METHOD(test_data_fixture, "lzss_encoder_test")
{
    agbpack::lzss_encoder encoder;
    agbpack::lzss_decoder decoder;
    set_test_data_directory("lzss_encoder");

    SECTION("Successful encoding")
    {
        // TODO: add more tests as we develop the encoder
        const auto filename = GENERATE("lzss.good.zero-length-file.txt");
        const auto original_data = read_decoded_file(filename);

        // Encode
        const auto encoded_data = encode_vector(encoder, original_data);
        // TODO: check size of encoded data

        // Decode
        const auto decoded_data = decode_vector(decoder, encoded_data);
        CHECK(decoded_data == original_data);
    }
}

}

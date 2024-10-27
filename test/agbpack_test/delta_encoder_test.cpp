// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <stdexcept>
#include <string>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

using string = std::string;

TEST_CASE("delta_encoder_test")
{
    agbpack::delta_encoder encoder;
    test_data_directory test_data_directory("delta");

    SECTION("Successful 8 bit encoding")
    {
        const string filename = GENERATE(
            "delta.good.8.zero-length-file.txt",
            "delta.good.8.one-byte.txt",
            "delta.good.8.sine.bin");
        const auto expected_data = test_data_directory.read_encoded_file(filename);
        const auto decoded_data = test_data_directory.read_decoded_file(filename);

        encoder.options(agbpack::delta_options::delta8);
        const auto encoded_data = encode_vector(encoder, decoded_data);

        CHECK(encoded_data == expected_data);
    }

    SECTION("Successful 16 bit encoding")
    {
        // TODO: what if input is an odd number of bytes? => Well that's an error, innit? What do other encoders do in that case?
        const string filename = GENERATE(
            "delta.good.16.zero-length-file.txt",
            "delta.good.16.one-word.bin",
            "delta.good.16.sine.bin");
        const auto expected_data = test_data_directory.read_encoded_file(filename);
        const auto decoded_data = test_data_directory.read_decoded_file(filename);

        encoder.options(agbpack::delta_options::delta16);
        const auto encoded_data = encode_vector(encoder, decoded_data);

        CHECK(encoded_data == expected_data);
    }

    SECTION("Invalid options")
    {
        CHECK_THROWS_MATCHES(
            encoder.options(agbpack::delta_options(-1)),
            std::invalid_argument,
            Catch::Matchers::Message("invalid delta compression options"));
    }

    SECTION("Input data too big")
    {
        const std::vector<unsigned char> input(agbpack::maximum_uncompressed_size + 1);

        CHECK_THROWS_AS(encode_vector(encoder, input), agbpack::encode_exception);
    }
}

}

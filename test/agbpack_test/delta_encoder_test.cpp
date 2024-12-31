// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <stdexcept>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

TEST_CASE_METHOD(test_data_fixture, "delta_encoder_test")
{
    agbpack::delta_encoder encoder;
    set_test_data_directory("delta");

    SECTION("Successful 8 bit encoding")
    {
        const auto filename = GENERATE(
            "delta.good.8.zero-length-file.txt",
            "delta.good.8.one-byte.txt",
            "delta.good.8.sine.bin");
        const auto expected_encoded_data = read_encoded_file(filename);

        encoder.options(agbpack::delta_options::delta8);
        const auto encoded_data = encode_file(encoder, filename);

        CHECK(encoded_data == expected_encoded_data);
    }

    SECTION("Successful 16 bit encoding")
    {
        // TODO: what if input is an odd number of bytes? => Well that's an error, innit? What do other encoders do in that case?
        const auto filename = GENERATE(
            "delta.good.16.zero-length-file.txt",
            "delta.good.16.one-word.bin",
            "delta.good.16.sine.bin");
        const auto expected_encoded_data = read_encoded_file(filename);

        encoder.options(agbpack::delta_options::delta16);
        const auto encoded_data = encode_file(encoder, filename);

        CHECK(encoded_data == expected_encoded_data);
    }

    SECTION("Invalid options")
    {
        CHECK_THROWS_MATCHES(
            encoder.options(agbpack::delta_options(-1)),
            std::invalid_argument,
            Catch::Matchers::Message("invalid delta compression options"));
    }
}

}

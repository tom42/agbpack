// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <stdexcept>

import agbpack;

namespace agbpack_test
{

// TODO: add test: successful 8 bit encoding
// TODO: add test: successful 4 bit encoding
// TODO: add test: input data too big
TEST_CASE("huffman_encoder_test")
{
    agbpack::huffman_encoder encoder;

    SECTION("Invalid options")
    {
        CHECK_THROWS_MATCHES(
            encoder.options(agbpack::huffman_options(-1)),
            std::invalid_argument,
            Catch::Matchers::Message("invalid huffman compression options"));
    }
}

}

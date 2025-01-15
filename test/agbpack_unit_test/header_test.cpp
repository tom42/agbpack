// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <cstddef>
#include <stdexcept>

import agbpack;
import agbpack_unit_testkit;

namespace agbpack_unit_test
{

using agbpack::encode_exception;
using agbpack::header;
using agbpack::huffman_options;
using agbpack::maximum_uncompressed_size;

TEST_CASE("header_test")
{
    SECTION("create, valid size")
    {
        auto valid_size = GENERATE(0u, maximum_uncompressed_size);

        CHECK(header::create(huffman_options::h8, valid_size).uncompressed_size() == valid_size);
    }

    SECTION("create, invalid size")
    {
        // TODO: this is not going to work on 32 bit targets. What to do?

        // Ideally we would use size_t literal suffixes here ('uz').
        // Unfortunately VS2022 does not have them yet.
        std::size_t invalid_size = GENERATE(maximum_uncompressed_size + 1ull, 0x100000000ull);

        CHECK_THROWS_MATCHES(
            header::create(huffman_options::h8, invalid_size),
            encode_exception,
            Catch::Matchers::Message("data to encode is too big"));
    }

    SECTION("to_uint32_t")
    {
        auto header = header::create(huffman_options::h8, 0x123456);

        CHECK(header.to_uint32_t() == 0x12345628);
    }
}

}

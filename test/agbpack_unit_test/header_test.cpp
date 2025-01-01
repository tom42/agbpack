// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

import agbpack;

namespace agbpack_unit_test
{

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

    SECTION("to_uint32_t")
    {
        auto header = header::create(huffman_options::h8, 0x123456);

        CHECK(header.to_uint32_t() == 0x12345628);
    }
}

}

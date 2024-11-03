// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import agbpack;

namespace agbpack_unit_test
{

using agbpack::header;
using agbpack::huffman_options;

TEST_CASE("header_test")
{
    SECTION("to_uint32_t")
    {
        auto header = header::create(huffman_options::h8, 0x123456);

        CHECK(header.to_uint32_t() == 0x12345628);
    }
}

}

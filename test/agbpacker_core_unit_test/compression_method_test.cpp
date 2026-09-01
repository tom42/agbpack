// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import agbpacker_core;

namespace agbpacker_core_unit_test
{

using agbpacker_core::compression_method;
using agbpacker_core::find_compression_method;

TEST_CASE("compression_method_test")
{
    SECTION("find_compression_method")
    {
        CHECK(find_compression_method("lzss")->method == compression_method::lzss);
        CHECK(find_compression_method("d16")->method == compression_method::d16);
        CHECK(find_compression_method("unknown_method") == nullptr);
    }
}

}
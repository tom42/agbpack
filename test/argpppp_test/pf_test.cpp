// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import argpppp;

namespace argpppp_unit_test
{

using argpppp::pf;

TEST_CASE("pf_test")
{
    SECTION("bitwise or")
    {
        constexpr auto result = pf::parse_argv0 | pf::long_only;
        CHECK(result == pf(0x41));
    }

    SECTION("to_uint")
    {
        constexpr auto result = to_uint(pf::silent);
        CHECK(result == 0x32);
    }
}

}

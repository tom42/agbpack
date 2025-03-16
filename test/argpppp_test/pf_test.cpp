// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import argpppp;

// TODO: should this not be argpppp_unit_test?
namespace argpppp_test
{

using argpppp::pf;

TEST_CASE("pf_test")
{
    SECTION("bitwise or")
    {
        constexpr auto result = pf::parse_argv0 | pf::long_only;
        CHECK(result == pf(0x41));
    }
}

}

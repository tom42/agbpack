// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import argpppp;

namespace argpppp_test
{

using argpppp::of;

TEST_CASE("of_test")
{
    SECTION("operator_bitwise_or_test")
    {
        constexpr auto result = of::arg_optional | of::alias;
        CHECK(result == of(5));
    }

    SECTION("to_int_test")
    {
        constexpr auto result = to_int(of::no_usage);
        CHECK(result == 16);
    }
}

}

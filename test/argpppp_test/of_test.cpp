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
        CHECK((of::arg_optional | of::alias) == of(5));
    }

    SECTION("to_int_test")
    {
        CHECK(to_int(of::no_usage) == 16);
    }
}

}

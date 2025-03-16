// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <cstring>

import argpppp;

namespace argpppp_unit_test
{

using argpppp::c_str;
using argpppp::optional_string;

TEST_CASE("optional_string_test")
{
    SECTION("c_str, optional_string contains no string")
    {
        const optional_string os;
        CHECK(c_str(os) == nullptr);
    }

    SECTION("c_str, optional_string contains string")
    {
        const optional_string os("string");
        CHECK(std::strcmp(c_str(os), "string") == 0);
    }
}

}

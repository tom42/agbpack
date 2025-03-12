// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import argpppp;

namespace argpppp_test
{

using argpppp::option_callback_result;

TEST_CASE("option_callback_result_test")
{
    SECTION("constructor, construction from true")
    {
        constexpr option_callback_result result = true;
        CHECK(result.is_successful() == true);
        CHECK(result.error_message().has_value() == false);
    }

    SECTION("constructor, construction from false")
    {
        constexpr option_callback_result result = false;
        CHECK(result.is_successful() == false);
        CHECK(result.error_message().has_value() == false);
    }

    SECTION("constructor, construction from string")
    {
        constexpr option_callback_result result = "123"; // TODO: whoops: that uses implicit conversion to bool
        CHECK(result.is_successful() == false); // TODO: whoops: that fails
        // TODO: also test error_message()

        // TODO: also test the error helper function
    }
}

}

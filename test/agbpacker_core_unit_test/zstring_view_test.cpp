// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <string>

import agbpacker_core;

namespace agbpacker_core_unit_test
{

using agbpacker_core::zstring_view;

TEST_CASE("zstring_view_test")
{
    SECTION("construction from const char*")
    {
        const char* underlying = "abc";
        zstring_view view(underlying);

        CHECK(std::strcmp(view.c_str(), "abc") == 0);
    }

    SECTION("construction from std::string")
    {
        std::string underlying("abcd");
        zstring_view view(underlying);

        CHECK(std::strcmp(view.c_str(), "abcd") == 0);
    }
}

}
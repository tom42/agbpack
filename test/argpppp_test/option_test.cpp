// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import argpppp;

namespace argpppp_test
{

using argpppp::of;
using argpppp::option;

TEST_CASE("option_test")
{
    SECTION("constructor, all arguments given")
    {
        const auto opt = option("name", 'n', "arg", of::arg_optional, "doc", 123);
        CHECK(opt.name() == "name");
        CHECK(opt.key() == 'n');
        CHECK(opt.arg() == "arg");
        CHECK(opt.flags() == of::arg_optional);
        CHECK(opt.doc() == "doc");
        CHECK(opt.group() == 123);
    }
}

}

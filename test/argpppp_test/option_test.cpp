// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <optional>

import argpppp;

namespace argpppp_test
{

using argpppp::of;
using argpppp::option;
using argpppp::optional_string;
using std::nullopt;

TEST_CASE("option_test")
{
    SECTION("constructor, all arguments specified")
    {
        const option opt("name", 'n', "arg", of::arg_optional, "doc", 123);
        CHECK(opt.name() == "name");
        CHECK(opt.key() == 'n');
        CHECK(opt.arg() == "arg");
        CHECK(opt.flags() == of::arg_optional);
        CHECK(opt.doc() == "doc");
        CHECK(opt.group() == 123);
    }

    SECTION("constructor, all arguments use default values")
    {
        const option opt;
        CHECK(opt.name() == nullopt);
        CHECK(opt.key() == 0);
        CHECK(opt.arg() == nullopt);
        CHECK(opt.flags() == of::none);
        CHECK(opt.doc() == nullopt);
        CHECK(opt.group() == 0);
    }

    SECTION("to_argp_option")
    {
        const option opt("name", 'n', "arg", of::arg_optional, "doc", 123);
        const auto argp_option = to_argp_option(opt);
        CHECK(!strcmp(argp_option.name, "name"));
        CHECK(argp_option.key == 'n');
        CHECK(!strcmp(argp_option.arg, "arg"));
        CHECK(argp_option.flags == to_int(of::arg_optional));
        CHECK(!strcmp(argp_option.doc, "doc"));
        CHECK(argp_option.group == 123);
    }
}

}

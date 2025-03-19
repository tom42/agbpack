// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <limits>
#include <optional>
#include <vector>

import argpppp;

namespace argpppp_unit_test
{

using argpppp::is_printable_key;
using argpppp::need_long_name;
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

    SECTION("constructor throws if a long name is required but not given")
    {
        CHECK(option({}, 'x').name().has_value() == false);
        CHECK_THROWS_MATCHES(
            option({}, 256),
            std::invalid_argument,
            Catch::Matchers::Message("option without printable short name needs a long name"));
    }

    SECTION("is_printable_key")
    {
        CHECK(is_printable_key(std::numeric_limits<int>::min()) == false);
        CHECK(is_printable_key(-1) == false);
        CHECK(is_printable_key(0) == false);
        CHECK(is_printable_key(31) == false);
        CHECK(is_printable_key(32) == true);
        CHECK(is_printable_key(126) == true);
        CHECK(is_printable_key(127) == false);
        CHECK(is_printable_key(std::numeric_limits<int>::max()) == false);
    }

    SECTION("need_long_name")
    {
        CHECK(need_long_name(0) == false);
        CHECK(need_long_name(1) == true);
        CHECK(need_long_name('a') == false);
    }

    SECTION("get_names")
    {
        CHECK(get_names(option({}, 's')) == "'-s'");
        CHECK(get_names(option("long-name")) == "'--long-name'");
        CHECK(get_names(option("long-name", 's')) == "'-s' / '--long-name'");
        CHECK_THROWS_MATCHES(
            get_names(option()),
            std::invalid_argument,
            Catch::Matchers::Message("get_names: option has no name"));
    }

    SECTION("get_default_error_message")
    {
        // TODO: tests:
        //       * Argument is 0
        //       * Option with optional argument: no special case for the moment I guess
        //       * Option without argument (other test?)
        CHECK(get_default_error_message(option({}, 'x'), "foo") == "invalid argument 'foo' for option '-x'");
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

    SECTION("to_argp_options")
    {
        const std::vector<option> options
        {
            {"option1", 'o', "arg1", of::arg_optional, "doc1", 1},
            {"option2", 'p', "arg2", of::no_usage, "doc2", 2}
        };

        const auto argp_options = to_argp_options(options);

        CHECK(argp_options.size() == 3);

        CHECK(!strcmp(argp_options[0].name, "option1"));
        CHECK(argp_options[0].key == 'o');
        CHECK(!strcmp(argp_options[0].arg, "arg1"));
        CHECK(argp_options[0].flags == to_int(of::arg_optional));
        CHECK(!strcmp(argp_options[0].doc, "doc1"));
        CHECK(argp_options[0].group == 1);

        CHECK(!strcmp(argp_options[1].name, "option2"));
        CHECK(argp_options[1].key == 'p');
        CHECK(!strcmp(argp_options[1].arg, "arg2"));
        CHECK(argp_options[1].flags == to_int(of::no_usage));
        CHECK(!strcmp(argp_options[1].doc, "doc2"));
        CHECK(argp_options[1].group == 2);

        CHECK(argp_options[2].name == nullptr);
        CHECK(argp_options[2].key == 0);
        CHECK(argp_options[2].arg == nullptr);
        CHECK(argp_options[2].flags == 0);
        CHECK(argp_options[2].doc == nullptr);
        CHECK(argp_options[2].group == 0);
    }
}

}

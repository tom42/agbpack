// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <cerrno>
#include <cstring>
#include <functional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

import argpppp;

namespace argpppp_unit_test
{

using argpppp::arg_error;
using argpppp::of;
using argpppp::parser;
using argpppp::pf;
using string = std::string;
using std::vector;

namespace
{

template <typename Iterator>
std::vector<char> make_arg(Iterator begin, Iterator end)
{
    std::vector<char> arg(begin, end);
    arg.push_back(0); // Add terminating zero
    return arg;
}

std::vector<char> make_arg(const char* s)
{
    return make_arg(s, s + strlen(s));
}

auto parse(parser& parser, const std::string& command_line)
{
    // 1) Build vector of zero terminated arguments.
    std::vector<std::vector<char>> args;
    args.push_back(make_arg("program_name"));
    for (auto word : std::views::split(command_line, ' '))
    {
        args.push_back(make_arg(word.begin(), word.end()));
    }

    // 2) Build argv, a vector containing char pointers to the zero terminated arguments
    std::vector<char*> argv;
    for (auto& arg : args)
    {
        argv.push_back(arg.data());
    }

    return parser.parse(static_cast<int>(argv.size()), argv.data(), pf::no_exit);
}

}

TEST_CASE("parser_test")
{
    parser parser;
    std::string failure_message;
    parser.set_failure_callback([&failure_message](int, const std::string& msg) { failure_message += msg; });

    SECTION("add_option throws if an option with key = 0 has a callback")
    {
        CHECK_THROWS_MATCHES(
            add_option(parser, { "This is a documentation option", {}, {}, of::doc }, [](auto){ return true; } ),
            std::invalid_argument,
            Catch::Matchers::Message("add_option: special options with key = 0 must not have callbacks"));
    }

    SECTION("add_option throws if an option with key != 0 does not have a callback")
    {
        CHECK_THROWS_MATCHES(
            add_option(parser, { {}, 'a' }, {}),
            std::invalid_argument,
            Catch::Matchers::Message("add_option: option must have a callback"));
    }

    SECTION("Exceptions abort parsing and are propagated to caller")
    {
        add_option(parser, { {}, 'a' }, [](auto)->bool{ throw std::runtime_error("This exception should occur."); });
        add_option(parser, { {}, 'b' }, [](auto)->bool{ throw std::runtime_error("This exception should not occur."); });

        CHECK_THROWS_MATCHES(
            parse(parser, "-a -b"),
            std::runtime_error,
            Catch::Matchers::Message("This exception should occur."));
    }

    SECTION("Successful parsing of switches")
    {
        bool a_seen = false;
        bool b_seen = false;
        bool c_seen = false;

        add_option(parser, { {}, 'a' }, [&](auto){ return a_seen = true; });
        add_option(parser, { {}, 'b' }, [&](auto){ return b_seen = true; });
        add_option(parser, { {}, 'c' }, [&](auto){ return c_seen = true; });

        auto result = parse(parser, "-c -a");

        CHECK(result.errnum == 0);
        CHECK(failure_message == "");
        CHECK(a_seen == true);
        CHECK(b_seen == false);
        CHECK(c_seen == true);
    }

    SECTION("Parsing should stop if an option callback returns false")
    {
        bool a_seen = false;

        add_option(parser, { {}, 'a' }, [&](auto){ a_seen = true; return false; });
        add_option(parser, { {}, 'b' }, [](auto)->bool{ throw std::runtime_error("This exception should not occur."); });

        auto result = parse(parser, "-a -b");

        CHECK(result.errnum == EINVAL);
        CHECK(failure_message == "unexpected option '-a'");
        CHECK(a_seen == true);
    }

    SECTION("Parsing should stop if an option callback returns arg_error")
    {
        bool a_seen = false;

        add_option(parser, { {}, 'a' }, [&](auto) { a_seen = true; return arg_error("custom error message"); });
        add_option(parser, { {}, 'b' }, [](auto)->bool { throw std::runtime_error("This exception should not occur."); });

        auto result = parse(parser, "-a -b");

        CHECK(result.errnum == EINVAL);
        CHECK(failure_message == "custom error message");
        CHECK(a_seen == true);
    }

    SECTION("Unlimited number of arguments")
    {
        auto result = parse(parser, "arg1 arg2 arg3 arg4");

        CHECK(result.errnum == 0);
        CHECK(result.args == vector<string>{"arg1", "arg2", "arg3", "arg4"});
        CHECK(failure_message == "");
    }

    SECTION("Too few arguments")
    {
        parser.set_nargs(2);

        auto result = parse(parser, "x");

        CHECK(result.errnum == EINVAL);
        CHECK(result.args == vector<string>{"x"});
        CHECK(failure_message == "too few arguments");
    }

    SECTION("Too many arguments")
    {
        parser.set_nargs(2);

        auto result = parse(parser, "x y z");

        CHECK(result.errnum == EINVAL);
        CHECK(result.args == vector<string>{"x", "y", "z"});
        CHECK(failure_message == "too many arguments");
    }

    SECTION("Correct number of arguments")
    {
        parser.set_nargs(2);

        auto result = parse(parser, "x y");

        CHECK(result.errnum == 0);
        CHECK(result.args == vector<string>{"x", "y"});
        CHECK(failure_message == "");
    }

    SECTION("Minimum and maximum number of arguments differ")
    {
        parser.set_nargs(2, 3);

        CHECK(parse(parser, "1").errnum == EINVAL);
        CHECK(parse(parser, "1 2").errnum == 0);
        CHECK(parse(parser, "1 2 3").errnum == 0);
        CHECK(parse(parser, "1 2 3 4").errnum == EINVAL);
    }
}

}

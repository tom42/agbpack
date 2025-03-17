// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <cstring>
#include <functional>
#include <ranges>
#include <stdexcept>
#include <vector>

import argpppp;

namespace argpppp_unit_test
{

using argpppp::of;
using argpppp::parser;
using argpppp::pf;

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

void parse(parser& parser, const std::string& command_line)
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

    parser.parse(static_cast<int>(argv.size()), argv.data(), pf::no_exit);
}

}

TEST_CASE("parser_test")
{
    parser parser;

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

        // TODO: what would be the return code of parse here?
        parse(parser, "-c -a");

        CHECK(a_seen == true);
        CHECK(b_seen == false);
        CHECK(c_seen == true);
    }

    SECTION("Parsing should stop if an option callback returns false")
    {
        bool a_seen = false;

        add_option(parser, { {}, 'a' }, [&](auto){ a_seen = true; return false; });
        add_option(parser, { {}, 'b' }, [](auto)->bool{ throw std::runtime_error("This exception should not occur."); });

        // TODO: what would be the return code of parse here?
        parse(parser, "-a -b");

        CHECK(a_seen == true);
    }
}

}

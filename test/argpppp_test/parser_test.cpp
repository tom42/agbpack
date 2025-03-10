// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <cstring>
#include <functional> // TODO: this is required for g++, but not clang and MSVC. Why?
#include <ranges>
#include <stdexcept>
#include <vector>

import argpppp;

namespace argpppp_test
{

namespace
{

std::vector<char> make_arg(const char* s)
{
    // Copy including terminating zero
    return std::vector<char>(s, s + strlen(s) + 1);
}

void parse(argpppp::parser& parser, const std::string& command_line)
{
    // 1) Build vector of zero terminated arguments.
    std::vector<std::vector<char>> args;
    args.push_back(make_arg("program_name"));
    for (auto word : std::views::split(command_line, ' '))
    {
        // TODO: can we somehow use make_arg above for this?
        std::vector<char> arg(word.begin(), word.end());
        arg.push_back(0); // Add teminating zero
        args.push_back(arg);
    }

    // 2) Build argv, a vector containing char pointers to the zero terminated arguments
    std::vector<char*> argv;
    for (auto& arg : args)
    {
        argv.push_back(arg.data());
    }

    parser.parse(static_cast<int>(argv.size()), argv.data());
}

}

TEST_CASE("parser_test")
{
    argpppp::parser parser;

    SECTION("Exceptions abort parsing and are propagated to caller")
    {
        parser.add_option({ {}, 'a' }, []{ throw std::runtime_error("This exception should occur."); });
        parser.add_option({ {}, 'b' }, []{ throw std::runtime_error("This exception should not occur."); });

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

        parser.add_option({ {}, 'a' }, [&]() noexcept { a_seen = true; });
        parser.add_option({ {}, 'b' }, [&]() noexcept { b_seen = true; });
        parser.add_option({ {}, 'c' }, [&]() noexcept { c_seen = true; });

        // TODO: what would be the return code of parse here?
        parse(parser, "-c -a");

        CHECK(a_seen == true);
        CHECK(b_seen == false);
        CHECK(c_seen == true);
    }
}

}

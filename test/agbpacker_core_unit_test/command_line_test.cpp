// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <ranges>
#include <string_view>
#include <vector>

import agbpacker_core;

namespace agbpacker_core_unit_test
{

using agbpacker_core::parse_command_line_result;
using std::string_view;
using std::vector;

namespace
{

vector<char> make_arg(string_view s)
{
    vector<char> arg;
    arg.reserve(s.size() + 1);
    arg.append_range(s);
    arg.push_back(0); // Add terminating zero
    return arg;
}

class command_line_fixture
{
protected:
    parse_command_line_result parse_command_line(string_view command_line)
    {
        // Build vector of zero terminated arguments
        // Split at space characters without quoting, arguments containing spaces are therefore not supported.
        // This is sufficient for our test cases, quoting and such are low level details handled by getopt.
        vector<vector<char>> args;
        args.push_back(make_arg("program_name"));
        // TODO: split and add args
        for (auto arg : std::views::split(command_line, ' '))
        {
            // TODO: yikes: does not build
            //make_arg(arg);
        }

        // Build argv, a vector containing char pointers to the zero terminated arguments
        vector<char*> argv;
        for (auto& arg : args)
        {
            argv.push_back(arg.data());
        }

        return agbpacker_core::parse_command_line(static_cast<int>(argv.size()), argv.data(), true);
    }
};

}

TEST_CASE_METHOD(command_line_fixture, "command_line_test")
{
    SECTION("No input file")
    {
        auto result = parse_command_line("");

        CHECK(result.success == false);
    }

    SECTION("More than one input file")
    {
        auto result = parse_command_line("file1 file2");

        CHECK(result.success == false);
    }

    SECTION("Input file given without any options")
    {
        auto result = parse_command_line("file");

        // TODO: check file name
        CHECK(result.success == true);
    }
}

}

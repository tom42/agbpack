// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <ranges>
#include <string_view>
#include <utility>
#include <vector>

import agbpacker_core;

namespace agbpacker_core_unit_test
{

using agbpacker_core::compression_method;
using agbpacker_core::program_mode;
using agbpacker_core::parse_command_line_result;
using std::make_pair;
using std::string_view;
using std::vector;

namespace
{

vector<char> make_arg(string_view s)
{
    vector<char> arg;
    arg.reserve(s.size() + 1);
#ifdef __cpp_lib_containers_ranges
    arg.append_range(s);
#else
    arg.insert(arg.end(), s.begin(), s.end()); // For clang 18, which we still use on github.
#endif
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
        for (auto arg : std::views::split(command_line, ' '))
        {
            args.push_back(make_arg(string_view(arg)));
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

        CHECK(result.success == true);
        CHECK(result.input_file == "file");
        CHECK(result.output_file == "file");
        CHECK(result.mode == program_mode::compress);
        CHECK(result.method == compression_method::lzss);
    }

    SECTION("Output file given")
    {
        auto command_line = GENERATE(
            "file.input -o file.output",
            "-o file.output file.input");

        auto result = parse_command_line(command_line);

        CHECK(result.success == true);
        CHECK(result.input_file == "file.input");
        CHECK(result.output_file == "file.output");
    }

    SECTION("--compress option")
    {
        auto result = parse_command_line("-c file");

        CHECK(result.success == true);
        CHECK(result.input_file == "file");
        CHECK(result.mode == program_mode::compress);
    }

    SECTION("--decompress option")
    {
        auto result = parse_command_line("-d file");

        CHECK(result.success == true);
        CHECK(result.input_file == "file");
        CHECK(result.mode == program_mode::decompress);
    }

    SECTION("--compress and --decompress option")
    {
        auto [command_line, expected_mode] = GENERATE(
            make_pair("-c -d file", program_mode::decompress),
            make_pair("-d -c file", program_mode::compress));

        auto result = parse_command_line(command_line);

        CHECK(result.success == true);
        CHECK(result.input_file == "file");
        CHECK(result.mode == expected_mode);
    }

    SECTION("--compress option with compression method argument")
    {
        // TODO: check also different compression method
        // TODO: check also unknown compression method (possibly in a different test)
        auto result = parse_command_line("-c=lzss file");

        CHECK(result.success == true);
        CHECK(result.method == compression_method::lzss);
    }
}

}

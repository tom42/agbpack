// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cstring> // TODO: see whether to remove this once we've fully implemented compression mode parsing
#include <format>
#include <functional> // Required by g++ 15.2
#include <ranges>

module agbpacker_core;
import argpppp;

namespace agbpacker_core
{

using argpppp::callback;
using argpppp::command_line_parser;
using argpppp::error;
using argpppp::of;
using argpppp::ok;
using argpppp::option; // TODO: remove if not needed
using argpppp::option_occurrence;
using argpppp::options;
using argpppp::pf;
using argpppp::value;
using std::format;
using std::string;
using std::string_view;

namespace
{

command_line_parser make_parser(bool is_unit_test)
{
    command_line_parser parser;

    if (is_unit_test)
    {
        parser.flags(pf::no_errs | pf::no_exit);
    }

    return parser;
}

string list_compression_methods()
{
    using namespace std::ranges;
    using namespace std::views;

    return all_compression_methods()
        | transform([](auto& method) { return string_view(method.name); })
        | join_with(string_view(", "))
        | to<string>();
}

}

parse_command_line_result parse_command_line(int argc, char* argv[], bool is_unit_test)
{
    parse_command_line_result result;

    auto parse_compression_method = [&](option_occurrence opt)
    {
            result.mode = program_mode::compress;

            if (opt.c_arg())
            {
                // TODO: no ad-hoc string parsing here - delegate to parsing method which knows about all compression methods
                if (!strcmp(opt.c_arg(), "lzss"))
                {
                    result.method = compression_method::lzss;
                }
                else if (!strcmp(opt.c_arg(), "rle"))
                {
                    result.method = compression_method::rle;
                }
                else
                {
                    return error(opt, "unknown compression method");
                }
            }

            return ok();
    };

    options command_line_options;
    command_line_options
        .doc("Compress and decompress data for the GBA BIOS\nhttps://github.com/tom42/agbpack\n\nData is LZSS compressed by default if neither of -c or -d is given.")
        .args_doc("FILE")
        .num_args(1)
        // TODO: consider having an overload of callback where args not need be given?
        //           * Question: should we have a special overload for add() that makes the callback() thing optional/redundant
        //             * Basically, special case callback, so that lambda expressions can be bassed to add and they get wrapped into a callback
        // TODO: obtain default compression method from constant, and use that to get the default compression method name
        .add({ 'c', "compress", format("Compress the input file using the specified compression method. Compression method defaults to 'lzss' if not given. Valid compression methods are: {}", list_compression_methods()), "METHOD", of::arg_optional }, callback(parse_compression_method))
        .add({ 'd', "decompress", "Decompress the input file" }, callback([&] { result.mode = program_mode::decompress; return ok(); }))
        .add({ 'o', "output-file", "Output file name. If not given, input file is overwritten", "FILE" }, value(result.output_file))
        .add({ {}, "vram-safe", "Use VRAM safe version of compression method if available" }, value(result.vram_safe));

    auto parser = make_parser(is_unit_test);
    auto parse_result = parser.parse(argc, argv, command_line_options);

    result.success = parse_result.errnum == 0;

    if (result.success)
    {
        result.input_file = parse_result.args.at(0);
        if (result.output_file.empty())
        {
            result.output_file = result.input_file;
        }
    }

    return result;
}

}

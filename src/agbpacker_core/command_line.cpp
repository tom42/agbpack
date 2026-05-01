// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <functional> // Required by g++ 15.2
#include <cstring> // TODO: see whether to remove this once we've fully implemented compression mode parsing

module agbpacker_core;
import argpppp;

namespace agbpacker_core
{

using argpppp::callback;
using argpppp::command_line_parser;
using argpppp::error;
using argpppp::of;
using argpppp::ok;
using argpppp::option;
using argpppp::options;
using argpppp::pf;
using argpppp::value;

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

}

parse_command_line_result parse_command_line(int argc, char* argv[], bool is_unit_test)
{
    parse_command_line_result result;

    auto parse_compression_method = [&](const option& opt, const char* arg)
    {
            result.mode = program_mode::compress;

            if (arg)
            {
                // TODO: no ad-hoc string parsing here - delegate to parsing method which knows about all compression methods
                if (!strcmp(arg, "lzss"))
                {
                    result.method = compression_method::lzss;
                }
                else if (!strcmp(arg, "rle"))
                {
                    result.method = compression_method::rle;
                }
                else
                {
                    return error(opt, arg, "unknown compression method");
                }
            }

            return ok();
    };

    options command_line_options;
    command_line_options
        .doc("Compress and decompress data for the GBA BIOS\nhttps://github.com/tom42/agbpack")
        .args_doc("FILE")
        .num_args(1)
        // TODO: consider having an overload of callback where args not need be given?
        //       * Yes but also consider order of args:
        //         * Currently its option struct first, then the option text
        //         * Consider switching this, on the basis that the other way round is more common
        //       * Other possibility: pass the option description and the value in a single parameter object
        //         * Benefit: we only ever need one or two overloads of callback()
        //           * One taking the parameter object. and this one we design such that declaring it as const auto& is acceptable good, and declaring it as just auto is still fine
        //           * One taking no arguments at all
        //           * Question: should we have a special overload for add() that makes the callback() thing optional/redundant
        //           * Question: use string_view rather than const char*? (OK, but what if it is empty? how can we distinguish between not there and there but empty string? Question is, do we need to make that distinction)
        // TODO: obtain default compression method from constant, and use that to get the default compression method name
        // TODO: create list of compression methods programmatically (it's not even correct yet)
        // TODO: should document somewhere that compression is the default (doc string?)
        // TODO: need a way to request vram safety (--vram / --no-vram - the latter is probably optional?)
        .add({ 'c', "compress", "Compress the input file using the specified compression method. Compression method defaults to 'lzss' if not given. Valid compression methods are: foo, bar, baz", "METHOD", of::arg_optional }, callback(parse_compression_method))
        .add({ 'd', "decompress", "Decompress the input file" }, callback([&](const option&, const char*) { result.mode = program_mode::decompress; return ok(); }))
        .add({ 'o', "output-file", "Output file name. If not given, input file is overwritten", "FILE" }, value(result.output_file));

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

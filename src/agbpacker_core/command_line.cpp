// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <functional> // Required by g++ 15.2

module agbpacker_core;
import argpppp;

namespace agbpacker_core
{

using argpppp::callback;
using argpppp::ok;
using argpppp::pf;
using argpppp::value;

namespace
{

argpppp::command_line_parser make_parser(bool is_unit_test)
{
    argpppp::command_line_parser parser;

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
    argpppp::options command_line_options;
    // TODO: compression mode: consider making this an optional argument of -c/--compress?
    command_line_options
        .doc("Compress and decompress data for the GBA BIOS\nhttps://github.com/tom42/agbpack")
        .args_doc("FILE")
        .num_args(1)
        // TODO: const/reference for args?
        // TODO: consider having an overload of callback where args not need be given?
        //       * Yes but also consider order of args:
        //         * Currently its option struct first, then the option text
        //         * Consider switching this, on the basis that the other way round is more common
        //       * Other possibility: pass the option description and the value in a single parameter object
        //         * Benefit: we only ever need one or two overloads of callback()
        //           * One taking the parameter object. and this one we design such that declaring it as const auto& is acceptable good, and declaring it as just auto is still fine
        //           * One taking no arguments at all
        // TODO: add optional argument specifying the compression mode => but then, what is the point of having a special overload for callback? => well for -d it'd still be totally awesome
        .add({ 'c', "compress", "Compress the input file" }, callback([&](const auto&, const auto&) { result.mode = program_mode::compress; return ok(); }))
        .add({ 'd', "decompress", "Decompress the input file" }, callback([&](const auto&, const auto&) { result.mode = program_mode::decompress; return ok(); }))
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

// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module agbpacker_core;
import argpppp;

namespace agbpacker_core
{

using argpppp::pf;

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
    argpppp::options command_line_options;
    // TODO: figure out to specify input and output:
    //       * 2 positional arguments: <input> <output>
    //       * 1 positional argument: <input>, output is optionally specified with option
    //           1a) If not given, output file name is same as input file name
    //           2a) If not given, output file name is somehow derived from input file name
    command_line_options
        .doc("Compress and decompress data for the GBA BIOS\nhttps://github.com/tom42/agbpack")
        .num_args(1);

    auto parser = make_parser(is_unit_test);
    auto parse_result = parser.parse(argc, argv, command_line_options);

    parse_command_line_result result;
    result.success = parse_result.errnum == 0;
    return result;
}

}

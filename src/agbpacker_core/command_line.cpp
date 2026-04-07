// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module agbpacker_core;
import argpppp;

namespace agbpacker_core
{

parse_command_line_result parse_command_line(int argc, char* argv[])
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
    auto parse_result = argpppp::parse_command_line(argc, argv, command_line_options);

    parse_command_line_result result;
    result.success = parse_result.errnum == 0;
    return result;
}

}

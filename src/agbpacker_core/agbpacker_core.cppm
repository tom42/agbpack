// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

export module agbpacker_core;

// TODO: possibly want to put command line parsing stuff into own partition
namespace agbpacker_core
{

enum class program_mode
{
    compress,
    decompress
};

export struct parse_command_line_result final
{
    program_mode mode = program_mode::compress;
    bool success = false;
};

export parse_command_line_result parse_command_line(int argc, char* argv[]);

}

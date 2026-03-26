// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

export module agbpacker_core;

// TODO: possibly want to put command line parsing stuff into own partition
namespace agbpacker_core
{

export struct parse_command_line_result final
{
    bool success{};
};

export parse_command_line_result parse_command_line(int argc, const char* argv[]);

}

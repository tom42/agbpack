// SPDX-FileCopyrightText: 2026 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <string>

export module agbpacker_core;

namespace agbpacker_core
{

AGBPACK_EXPORT_FOR_UNIT_TESTING
enum class program_mode
{
    compress,
    decompress
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
enum class compression_method
{
    lzss,
    optimal_lzss,
    h4,
    h8,
    rle,
    delta8,
    delta16
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
struct parse_command_line_result final
{
    bool success = false;
    program_mode mode = program_mode::compress;
    compression_method method = compression_method::lzss;
    std::string input_file;
    std::string output_file;
};

export parse_command_line_result parse_command_line(int argc, char* argv[], bool is_unit_test = false);

}

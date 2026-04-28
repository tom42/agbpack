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
    // TODO: add all compression methods (how do we distinguish between greedy and optimal lzss?)
    //       * and what about 4/8 bit huffman?
    //       * and what about 8/16 bit delta?
    lzss
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

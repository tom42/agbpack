// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

import agbpack;

namespace
{

using string = std::string;

struct command_line_options final
{
    string mode;
    string input_file;
    string output_file;
};

command_line_options parse_command_line(int argc, char* argv[])
{
    if (argc != 4)
    {
        throw std::runtime_error("wrong arguments. Usage: lzss-encode <mode> <input> <output>");
    }

    return command_line_options{argv[1], argv[2], argv[3]};
}

void encode(const command_line_options& /*options*/)
{
    // TODO: open/read file
    // TODO: encode file, using correct mode (normal/optimized)
    // TODO: write file
}

}

int main(int argc, char* argv[])
{
    try
    {
        auto options = parse_command_line(argc, argv);
        encode(options);
        // TODO: open/read file
        // TODO: encode file (can start with optimal for starters)
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << argv[0] << ": error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

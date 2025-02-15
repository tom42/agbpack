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
    string input_file;
};

command_line_options parse_command_line(int argc, char* argv[])
{
    // TODO: need 3/4 args here: mode, input file, output file
    if (argc != 2)
    {
        throw std::runtime_error("wrong number of arguments");
    }

    return command_line_options{argv[1]};
}

}

int main(int argc, char* argv[])
{
    try
    {
        auto options = parse_command_line(argc, argv);
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

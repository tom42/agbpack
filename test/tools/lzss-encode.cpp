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

}

int main(int /*argc*/, char* argv[])
{
    // TODO: implement LZSS encoding
    try
    {
        // TODO: parse command line
        // TODO: open/read file
        // TODO: encode file
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << argv[0] << ": error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

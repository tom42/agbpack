// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <iostream>
#include <stdexcept>
#include "agbpack_config.hpp"

import agbpack;
import argpppp;

#define PROGRAM_NAME "agbpacker"
extern "C" { const char* argp_program_version = PROGRAM_NAME " " AGBPACK_VERSION; }
static char program_name[] = PROGRAM_NAME;

import agbpacker_core;

namespace
{

using namespace agbpacker_core;

void run(const parse_command_line_result& options)
{
    // TODO: do something here
    switch (options.mode)
    {
        case program_mode::compress:
            break;
        case program_mode::decompress:
            break;
        default:
            throw std::logic_error("bad program mode");
    }
}

}

int main(int argc, char* argv[])
{
    try
    {
        argv[0] = program_name;
        auto options = agbpacker_core::parse_command_line(argc, argv);
        if (!options.success)
        {
            // Should not happen because we let argp_parse exit.
            return EXIT_FAILURE;
        }

        run(options);

        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << argv[0] << ": " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

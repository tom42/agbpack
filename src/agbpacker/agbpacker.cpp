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

void run(const agbpacker_core::parse_command_line_result& /*options*/) // TODO: I think we wanted to stick this into agbpacker_core, that's why that module does not export much
{
    // TODO: do something here
    throw std::runtime_error("YIKES");
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

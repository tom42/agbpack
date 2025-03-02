// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <iostream>
#include <stdexcept>

import argpppp;

// TODO: supply the real version from the agbpack version number. How do we get at that? macro or do we compile it into agbpack?
extern "C"
{
const char* argp_program_version = "agbpacker 0.3";
}

int main(int argc, char** argv)
{
    try
    {
        argpppp::parser parser;
        parser.doc("Compress and decompress data for the GBA BIOS");
        parser.parse(argc, argv);
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << argpppp::program_name(argv[0]) << ": " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <iostream>
#include <stdexcept>

import argpppp;

// TODO: test (remove, at least the bug address thing): does this work with argp-standalone, like this? Because if so we'll either
//       a) leave it at that
//       b) provide some API thrugh argpppp (but then...this are globals, and not our own...don't mess around with them)
// TODO: nope does not work => maybe we provide this inside argpppp, where we know whether glibc or argp-standalone is used?
// TODO: OK: With glibc/linux we need to *define* global variables like so:
// TODO: OK: if we use extern "C" like so, then it works. Question is then, do we want to abstract this in the library, or better not. Maybe better not? Seems to be fragile enough as it is?
extern "C"
{
    const char* argp_program_version = "0.3";
    const char* argp_program_bug_address = "/dev/null";
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

// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <iostream>
#include <stdexcept>

import agbpack;
import argpppp;

// TODO: supply the real version from the agbpack version number. How do we get at that? macro or do we compile it into agbpack?
//       => We create a configuration header and use that to bake the version into agbpack.
// TODO: problem: we do want to use agbpack::version here, but that has the value "0.0.1" and we also want to prepend "agbpacker"
//       => We have now multiple possibilities:
//          * We create a string at runtime, manage its lifetime ourselve and set it to arp_program_version (extern "C")
//          * We do the concatenation inside arg_program_verison hook. This requires lots of writing too
//          * We give in and come up with an api for more advanced program version settage (?)
//          * We give in and agbpacker gets its own configuration header. At this point it'd be the most sensible thing, really => least amount of work and it's gonna work
//          * Problem: how do we DO define the version number on the command line?
//          * If we go down this route, agbpack itself does NOT need a config header.
extern "C"
{
const char* argp_program_version = "agbpacker 0.3";
}

int main(int argc, char** argv)
{
    try
    {
        // TODO: rethink api
        //       Do we want to do like below, or do we want to have something like
        //       argpppp::options opts;
        //       add_option(opts, some_option(), &variable)
        //       add_option(opts, some_other_option(), &some_other_variable)
        //       => The procedural version leads to code that can be more naturally extended, no?
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

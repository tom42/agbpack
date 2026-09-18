// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <cstdio> // TODO: needed?
#include <iostream>
#include <stdexcept>
#include <vector>
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

// TODO: define a byte_vector?
std::vector<unsigned char> read_file(const std::string& filename)
{
    // TODO: open file (handle errors)
    // TODO: find fle size (handle errors)
    // TODO: read entire file (handle errors)
    // TODO: autoclose file, no error handling here
    // TODO: return data
    file::open(filename.c_str(), "rb");
    return {};
}

// TODO: might want to put this function into agbpacker_core and unit test it
void compress(const parse_command_line_result& options)
{
    // TODO: do something here (do not forget to honor all relevant options in that function)
    //       * method (e.g. lzss)
    //       * vram safety (if it applies)
    //       * input file
    //       * optional output file
    // TODO: so here is what we do:
    //       * read input file
    //         * So we need:
    //         * fopen, handle errors
    //         * Get file size, handle errors
    //         * fread, handle errors
    //         * fclose, handle errors (really? do we need this?) (yes, we do)
    //       * compress in-memory, take into account method and vram safety flag
    //       * write back to input file or output file if given
    auto data = read_file(options.input_file);
}

// TODO: might want to put this function into agbpacker_core and unit test it
void decompress(const parse_command_line_result& /*options*/)
{
    // TODO: do something here (do not forget to honor all relevant options in that function)
    //       * input file
    //       * output file
    // TODO: so here is what we do
    //       * read input file
    //       * decompress in-memory (method can be read from file itself)
    //       * write back to input file or output file if given
}

void run(const parse_command_line_result& options)
{
    switch (options.mode)
    {
        case program_mode::compress:
            compress(options);
            break;
        case program_mode::decompress:
            decompress(options);
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

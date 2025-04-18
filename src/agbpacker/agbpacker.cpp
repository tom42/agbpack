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

int main(int argc, char** argv)
{
    // TODO: test code, replace by original code below
    try
    {
        argv[0] = program_name;
        argpppp::parser parser;
        parser.set_doc("Compress and decompress data for the GBA BIOS\nhttps://github.com/tom42/agbpack");
        add_option(parser, {}, {}); // TODO: bogus option to fix g++ debug build linking problems. Replace by real options
        parser.parse(argc, argv);
    }
    catch (const std::exception& e)
    {
        std::cerr << argv[0] << ": " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

// TODO: get stuff below built again
/*
#include <functional>

namespace
{

enum class program_mode
{
    compress,
    decompress
};

class options final
{
public:
    program_mode mode = program_mode::compress;
};

options parse_command_line(int argc, char** argv)
{
    // TODO: so what options do we need here?
    //       -c => compress   \ These two override eachother, -c could be default
    //       -d => decompress /
    //       Some way to select the compression method
    //       * LZSS, normal
    //       * LZSS, vram safe
    //       * 4 bit huffman
    //       * 8 bit huffman
    //       * RLE
    //       * delta8
    //       * delta16
    //       * LZSS needs some more options: greedy/optimal/others
    //       Arguments
    //       * input file
    //       * output file
    options options;

    // TODO: re compression mode: we can do better, no?
    //       It would be more beautiful to accept only one of -c/-d
    //       Yeah damit, put that already requires us again to be able to print/return an error message, no?
    // TODO: actually parse and store compression method
    //       => For this, argpppp actually should process the return value of the lambda!
    // TODO: do we also expect the compression method for decompression? (Would be simpler, but also rather silly, no?)
    // TODO: enforce that there are exactly two arguments
    // TODO: there should be no method option. The method should be an argument of --compress. It could be optional, in which case we'd use LZSS.
    parser.set_args_doc("<input> <output>");
    add_option(parser, { "compress", 'c', {}, {}, "Files are compressed by default" }, [&options](auto){ options.mode = program_mode::compress; return true; });
    add_option(parser, { "decompress", 'd' }, [&options](auto){ options.mode = program_mode::decompress; return true; });
    add_option(parser, { "method", 'm', "method", {}, "Compression method, default is LZSS" }, [](auto){ return true; });
    parser.parse(argc, argv);

    return options;
}

}

int main(int argc, char** argv)
{
    try
    {
        argv[0] = program_name;
        auto options = parse_command_line(argc, argv);
        // TODO: test code below. Remove and implement compression/decompression insead
        std::cout << (options.mode == program_mode::compress ? "compress" : "decompress") << "\n";
        return EXIT_SUCCESS;
    }
}
*/

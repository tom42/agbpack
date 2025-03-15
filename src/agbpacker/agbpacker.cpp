// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <functional>
#include <iostream>
#include <stdexcept>
#include "agbpack_config.hpp"

import agbpack;
import argpppp;

extern "C"
{
const char* argp_program_version = "agbpacker " AGBPACK_VERSION;
}

namespace
{

enum class mode
{
    compress,
    decompress
};

class options final
{
public:
    mode mode = mode::compress;
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

    // TODO: actually parse and store compression method
    //       => For this, argpppp actually should process the return value of the lambda!
    // TODO: do we also expect the compression method for decompression? (Would be simpler, but also rather silly, no?)
    // TODO: enforce that there are exactly two arguments
    argpppp::parser parser;
    parser.doc("Compress and decompress data for the GBA BIOS");
    parser.args_doc("<input> <output>");
    add_option(parser, { "compress", 'c', {}, {}, "Files are compressed by default" }, [&options](auto){ options.mode = mode::compress; return true; });
    add_option(parser, { "decompress", 'd' }, [&options](auto){ options.mode = mode::decompress; return true; });
    add_option(parser, { "method", 'm', "method", {}, "Compression method, default is LZSS" }, [](auto){ return true; });
    parser.parse(argc, argv);

    return options;
}

}

int main(int argc, char** argv)
{
    try
    {
        auto options = parse_command_line(argc, argv);
        // TODO: test code below. Remove and implement compression/decompression insead
        std::cout << (options.mode == mode::compress ? "compress" : "decompress") << "\n";
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << argpppp::program_name(argv[0]) << ": " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

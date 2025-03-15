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

int main(int argc, char** argv)
{
    try
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
        argpppp::parser parser;
        parser.doc("Compress and decompress data for the GBA BIOS");
        parser.args_doc("<input> <output>");
        add_option(parser, { "compress", 'c', {}, {}, "Files are compressed by default" }, [](auto){ return true; });
        add_option(parser, { "decompress", 'd' }, [](auto){ return true; });
        parser.parse(argc, argv);
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << argpppp::program_name(argv[0]) << ": " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

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
        // TODO: rethink api
        //       Do we want to do like below, or do we want to have something like
        //       argpppp::options opts;
        //       add_option(opts, some_option(), &variable)
        //       add_option(opts, some_other_option(), &some_other_variable)
        //       => The procedural version leads to code that can be more naturally extended, no?
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
        // TODO: re argpppp: while it initially seemed to be funny it looks totally hideous. Maybe rename it to just argpp
        argpppp::parser parser;
        parser.doc("Compress and decompress data for the GBA BIOS");
        parser.add_option({ "compress", 'c', {}, {}, {}, {} });
        parser.add_option({ "decompress", 'd', {}, {}, {}, {} });
        // TODO: test options below, remove
        parser.add_option({ "This is a documentation option", {}, {}, argpppp::of::doc, {}, {} });
        parser.add_option({ {}, {}, {}, {}, "This is a group header", {} });
        parser.add_option({ "option-with-doc", 'o', {}, {}, "Here is a doc string", {} });
        parser.add_option({ "yodel", 'y', "loudness", argpppp::of::arg_optional | argpppp::of::no_usage, "Das Jodeldiplom", {}});
        parser.parse(argc, argv);
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << argpppp::program_name(argv[0]) << ": " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

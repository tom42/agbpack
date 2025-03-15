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
        parser.args_doc("arg1 arg2 ...");
        add_option(parser, { "compress", 'c' }, [](auto){ return true; });
        add_option(parser, { "decompress", 'd' }, [](auto){ return true; });
        // TODO: test options below, remove
        add_header(parser, "This is a group header");
        add_option(parser, { "option-with-doc", 'o', {}, {}, "Here is a doc string" }, [](auto){ return true; });
        add_option(parser, { "yodel", 'y', "loudness", argpppp::of::arg_optional | argpppp::of::no_usage, "Das Jodeldiplom" }, [](auto){ return true; });
        add_option(parser, { {}, 'u' }, [](auto){ std::cout << "u seen\n"; return true; });
        add_option(parser, { {}, 'v' }, [](auto){ std::cout << "v seen\n"; return false; });
        parser.parse(argc, argv);
        std::cout << "BYE\n";
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << argpppp::program_name(argv[0]) << ": " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

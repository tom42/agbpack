// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <iostream>
#include <stdexcept>
#include <system_error>
#include <variant>
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
using bytevector = std::vector<unsigned char>;

bytevector read_file(const std::string& filename)
{
    try
    {
        return file::read_all_bytes(filename);
    }
    catch (const std::system_error& e)
    {
        throw std::runtime_error("could not read " + filename + ": " + e.what());
    }
}

void write_file(const std::string& filename, const bytevector& data)
{
    try
    {
        file::write_all_bytes(filename, data);
    }
    catch (const std::system_error& e)
    {
        throw std::runtime_error("could not write " + filename + ": " + e.what());
    }
}

// TODO: obviously we need to return something polymorphic here
//       * Either define some sort of interface, or use a variant
// TODO: configure the encoder where applicable (vram safety)
// TODO: test whether vram_safe flag is applied:
//       * lzss
//       * optimal_lzss
agbpack::lzss_encoder create_encoder(compression_method method, bool vram_safe)
{
    switch (method)
    {
        case compression_method::lzss:
            return agbpack::lzss_encoder(vram_safe);
        // TODO: support all methods below here
        case compression_method::optimal_lzss:
        case compression_method::h4:
        case compression_method::h8:
        case compression_method::rle:
        case compression_method::d8:
        case compression_method::d16:
        default:
            throw std::invalid_argument("invalid compression method");
    }
}

// TODO: this has the same name as the function below, which is somewhat silly
// TODO: do we test whether e.g. gbacrusher can decompress our output?
bytevector compress(const bytevector& data, const parse_command_line_result& options)
{
    bytevector compressed_data;
    auto encoder = create_encoder(options.method, options.vram_safe);
    encoder.encode(data.begin(), data.end(), back_inserter(compressed_data));
    return compressed_data;
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
    //       * compress in-memory, take into account method and vram safety flag
    //       * write back to input file or output file if given
    auto uncompressed_data = read_file(options.input_file);
    auto compressed_data = compress(uncompressed_data, options);
    write_file(options.output_file, compressed_data);
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
            throw std::logic_error("invalid program mode");
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

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
using encoder = std::variant<
    agbpack::lzss_encoder,
    agbpack::optimal_lzss_encoder,
    agbpack::huffman_encoder,
    agbpack::rle_encoder,
    agbpack::delta_encoder>;

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

// TODO: do not forget to add tests for the custom ctors we added
// TODO: test whether vram_safe flag is applied:
//       * lzss
//       * optimal_lzss
// TODO: also pass vram_safe flags to decoder later (there it performs validation)
//       * Maybe we document this in the command line?
//       * Also, need to extend agbpack for this, both production code and unit tests
encoder create_encoder(compression_method method, bool vram_safe)
{
    switch (method)
    {
        case compression_method::lzss:
            return agbpack::lzss_encoder(vram_safe);
        case compression_method::optimal_lzss:
            return agbpack::optimal_lzss_encoder(vram_safe);
        case compression_method::h4:
            return agbpack::huffman_encoder(agbpack::huffman_options::h4);
        case compression_method::h8:
            return agbpack::huffman_encoder(agbpack::huffman_options::h8);
        case compression_method::rle:
            return agbpack::rle_encoder();
        case compression_method::d8:
            return agbpack::delta_encoder(agbpack::delta_options::delta8);
        case compression_method::d16:
            return agbpack::delta_encoder(agbpack::delta_options::delta16);
        default:
            throw std::invalid_argument("invalid compression method");
    }
}

// TODO: do we test whether e.g. gbacrusher can decompress our output?
bytevector compress(const bytevector& data, const parse_command_line_result& options)
{
    try
    {
        bytevector compressed_data;
        auto encoder = create_encoder(options.method, options.vram_safe);
        std::visit([&](auto&& e) { e.encode(data.begin(), data.end(), back_inserter(compressed_data)); }, encoder);
        return compressed_data;
    }
    catch (const agbpack::agbpack_exception& e)
    {
        throw std::runtime_error("could not compress " + options.input_file + ": " + e.what());
    }
}

void compress(const parse_command_line_result& options)
{
    auto uncompressed_data = read_file(options.input_file);
    auto compressed_data = compress(uncompressed_data, options);
    write_file(options.output_file, compressed_data);
}

void decompress(const parse_command_line_result& /*options*/)
{
    // TODO: so here is what we do
    //       * read input file
    //       * detect compression method
    //         * For that, the file must be at least 4 bytes in size (I think - look it up)
    //         * If so we can read those 4 bytes and try to parse them into a header (that may fail!)
    //         * In the case of success, move to next step
    //       * create decoder, decompress in-memory
    //       * write back to input file or output file if given
    // TODO: add exception handling similar to compress() above
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

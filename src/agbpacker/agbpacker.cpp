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
using byte_vector = std::vector<unsigned char>;
using encoder = std::variant<
    agbpack::lzss_encoder,
    agbpack::optimal_lzss_encoder,
    agbpack::huffman_encoder,
    agbpack::rle_encoder,
    agbpack::delta_encoder>;
using decoder = std::variant<
    agbpack::lzss_decoder,
    agbpack::huffman_decoder,
    agbpack::rle_decoder,
    agbpack::delta_decoder>;

byte_vector read_file(const std::string& filename)
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

void write_file(const std::string& filename, const byte_vector& data)
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

// TODO: test whether vram_safe flag is applied (both encoder and decoder)
//       * lzss
//       * optimal_lzss
// TODO: also pass vram_safe flags to decoder later (there it performs validation)
//       * Maybe we document this in the command line?
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

decoder create_decoder(agbpack::compression_type type, bool vram_safe)
{
    switch (type)
    {
        case agbpack::compression_type::lzss:
            return agbpack::lzss_decoder(vram_safe);
        case agbpack::compression_type::huffman:
            return agbpack::huffman_decoder();
        case agbpack::compression_type::rle:
            return agbpack::rle_decoder();
        case agbpack::compression_type::delta:
            return agbpack::delta_decoder();
        default:
            throw std::invalid_argument("invalid compression method");
    }
}

byte_vector encode(const byte_vector& data, const parse_command_line_result& options)
{
    try
    {
        byte_vector encoded_data;
        auto encoder = create_encoder(options.method, options.vram_safe);
        std::visit([&](auto&& e) { e.encode(data.begin(), data.end(), back_inserter(encoded_data)); }, encoder);
        return encoded_data;
    }
    catch (const agbpack::agbpack_exception& e)
    {
        throw std::runtime_error("could not compress " + options.input_file + ": " + e.what());
    }
}

void compress(const parse_command_line_result& options)
{
    auto uncompressed_data = read_file(options.input_file);
    auto compressed_data = encode(uncompressed_data, options);
    write_file(options.output_file, compressed_data);
}

agbpack::compression_type detect_compression_type(const byte_vector& compressed_data)
{
    auto header = agbpack::header::try_parse(compressed_data);
    if (!header)
    {
        // TODO: decode_exception adds some silly "encoded data is corrupt:" prefix to the message. Do we really need this? Here it is annoying
        throw agbpack::decode_exception(header.error());
    }

    return header.value().type();
}

byte_vector decode(const byte_vector& data, const parse_command_line_result& options)
{
    try
    {
        byte_vector decoded_data;
        auto compression_type = detect_compression_type(data);
        auto decoder = create_decoder(compression_type, options.vram_safe);
        std::visit([&](auto&& e) { e.decode(data.begin(), data.end(), back_inserter(decoded_data)); }, decoder);
        return decoded_data;
    }
    catch (const agbpack::agbpack_exception& e)
    {
        throw std::runtime_error("could not decompress " + options.input_file + ": " + e.what());
    }
}

void decompress(const parse_command_line_result& options)
{
    auto compressed_data = read_file(options.input_file);
    auto uncompressed_data = decode(compressed_data, options);
    write_file(options.output_file, uncompressed_data);
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

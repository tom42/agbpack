// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <system_error>
#include <string>

import agbpack;
import agbpack_test_tools_common;

namespace
{

using ifstream = std::ifstream;
using ofstream = std::ofstream;
using string = std::string;

enum class compression_mode
{
    normal,
    optimal
};

struct options final
{
    compression_mode mode;
    string input_file;
    string output_file;
};

compression_mode parse_compression_mode(const char* mode)
{
    if (!std::strcmp(mode, "normal"))
    {
        return compression_mode::normal;
    }
    else if(!std::strcmp(mode, "optimal"))
    {
        return compression_mode::optimal;
    }
    else
    {
        throw std::runtime_error(std::format("invalid compression mode '{}'", mode));
    }
}

options parse_command_line(int argc, char* argv[])
{
    if (argc != 4)
    {
        throw std::runtime_error("wrong arguments. Usage: lzss-encode <mode> <input> <output>");
    }

    auto mode = parse_compression_mode(argv[1]);
    return options{mode, argv[2], argv[3]};
}

template <typename TEncoder>
void encode(TEncoder& encoder, ifstream& input, ofstream& output)
{
    encoder.encode(
        std::istream_iterator<unsigned char>(input),
        std::istream_iterator<unsigned char>(),
        std::ostream_iterator<unsigned char>(output));
}

void encode_normal(ifstream& input, ofstream& output)
{
    agbpack::lzss_encoder encoder;
    encode(encoder, input, output);
}

void encode_optimal(ifstream& input, ofstream& output)
{
    agbpack::optimal_lzss_encoder encoder;
    encode(encoder, input, output);
}

void encode(compression_mode mode, ifstream& input, ofstream& output)
{
    switch (mode)
    {
        case compression_mode::normal:
            encode_normal(input, output);
            return;
        case compression_mode::optimal:
            encode_optimal(input, output);
            return;
    }

    throw std::invalid_argument("invalid compression mode");
}

void encode(const options& options)
{
    try
    {
        auto input = agbpack_test_tools_common::open_binary_input_file(options.input_file);
        auto output = agbpack_test_tools_common::open_binary_output_file(options.output_file);
        encode(options.mode, input, output);
    }
    catch (const ifstream::failure&)
    {
        // Depending on the library, exceptions thrown by ifstream may have rather useless error messages,
        // so we catch exceptions here and try our luck with errno and std::system_error instead.
        auto error = errno;
        throw std::system_error(error, std::generic_category(), "could not encode " + options.input_file);
    }
}

}

int main(int argc, char* argv[])
{
    try
    {
        auto options = parse_command_line(argc, argv);
        encode(options);
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << argv[0] << ": error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

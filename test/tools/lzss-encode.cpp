// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <system_error>
#include <string>

import agbpack;

namespace
{

using ifstream = std::ifstream;
using ofstream = std::ofstream;
using string = std::string;

struct command_line_options final
{
    string mode;
    string input_file;
    string output_file;
};

command_line_options parse_command_line(int argc, char* argv[])
{
    if (argc != 4)
    {
        throw std::runtime_error("wrong arguments. Usage: lzss-encode <mode> <input> <output>");
    }

    return command_line_options{argv[1], argv[2], argv[3]};
}

ifstream open_binary_input_file(const string& path)
{
    ifstream file;

    file.exceptions(ifstream::badbit | ifstream::eofbit | ifstream::failbit);
    file.open(path, std::ios_base::binary);

    // Set badbit only for processing.
    // Caution: I have no clue what I am doing here.
    file.exceptions(ifstream::badbit);
    file.unsetf(std::ios::skipws); // Required to correctly read binary files using some APIs, e.g. std::istream_iterator.
    return file;
}

ofstream open_binary_output_file(const string& path)
{
    ofstream file;

    file.exceptions(ifstream::badbit | ifstream::eofbit | ifstream::failbit);
    file.open(path, std::ios_base::binary);

    // Set badbit only for processing.
    // Caution: I have no clue what I am doing here.
    file.exceptions(ifstream::badbit);
    file.unsetf(std::ios::skipws); // Required to correctly read binary files using some APIs, e.g. std::istream_iterator.
    return file;
}

void encode(const command_line_options& options)
{
    try
    {
        auto input = open_binary_input_file(options.input_file);
        auto output = open_binary_output_file(options.output_file);
        // TODO: support normal/optimized mode
        agbpack::lzss_encoder encoder;
        encoder.encode(
            std::istream_iterator<unsigned char>(input),
            std::istream_iterator<unsigned char>(),
            std::ostream_iterator<unsigned char>(output));
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

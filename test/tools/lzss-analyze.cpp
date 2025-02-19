// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

import agbpack;

namespace
{

using byte_vector = std::vector<unsigned char>;
using ifstream = std::ifstream;
using size_t = std::size_t;
using string = std::string;
using std::format;

enum output_format_flags
{
    standard = 0,
    compressed_position = 1
};

struct options final
{
    string input_file;
    output_format_flags flags;
};

class debug_lzss_decoder_receiver final
{
public:
    explicit debug_lzss_decoder_receiver(std::ostream& os, output_format_flags flags)
        : m_flags(flags)
        , m_os(os)
    {}

    void tags(unsigned char tags)
    {
        m_os << format("{:#06x}", uncompressed_position());
        if (m_flags & output_format_flags::compressed_position)
        {
            m_os << format(" {:#06x}", m_compressed_position);
        }
        m_os << format(" T {:#010b} ({:#04x})\n", tags, tags);

        m_compressed_position += 1;
    }

    void literal(unsigned char c)
    {
        m_os << format("{:#06x}", uncompressed_position());
        if (m_flags & output_format_flags::compressed_position)
        {
            m_os << format(" {:#06x}", m_compressed_position);
        }
        m_os << format(" L '{}'\n", escape_character(c));

        m_uncompressed_data.push_back(c);
        m_compressed_position += 1;
    }

    void reference(size_t length, size_t offset)
    {
        size_t reference_position = m_uncompressed_data.size() - offset;
        m_os << format("{:#06x}", uncompressed_position());
        if (m_flags & output_format_flags::compressed_position)
        {
            m_os << format(" {:#06x}", m_compressed_position);
        }
        m_os << format(" R {:#04x} {:#05x} @{:#04x}'", length, offset, reference_position);

        while (length--)
        {
            unsigned char c = m_uncompressed_data[reference_position++];
            m_os << escape_character(c);
            m_uncompressed_data.push_back(c);
        }

        m_os << "'\n";
        m_compressed_position += 2;
    }

private:
    size_t uncompressed_position() const { return m_uncompressed_data.size(); }

    string escape_character(unsigned char byte)
    {
        if ((byte == '\\') || !std::isprint(byte))
        {
            return format("\\x{:02x}", byte);
        }

        return string(1, char(byte));
    }

    size_t m_compressed_position = 0;
    output_format_flags m_flags;
    std::ostream& m_os;
    byte_vector m_uncompressed_data;
};

output_format_flags parse_output_format(const char*)
{
    // TODO: somehow parse optional output format
    return {};
}

options parse_command_line(int argc, char* argv[])
{
    if ((argc < 2) || (argc > 3))
    {
        throw std::runtime_error("wrong arguments. Usage: lzss-analyze <input> [output format]");
    }

    output_format_flags output_format = output_format_flags::standard;
    if (argc == 3)
    {
        output_format = parse_output_format(argv[2]);
    }

    return options{argv[1], output_format };
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

void pad_data(byte_vector& data)
{
    while (data.size() % 4 != 0)
    {
        data.push_back(0);
    }
}

void analyze_file(const options& options)
{
    try
    {
        auto filestream = open_binary_input_file(options.input_file);

        // Some encoders do not correctly pad the file.
        // Our decoder does not like this, so we read the entire file and pad it if necessary.
        byte_vector data;
        std::copy(
            std::istream_iterator<unsigned char>(filestream),
            std::istream_iterator<unsigned char>(),
            back_inserter(data));
        pad_data(data);

        // Analyze file.
        agbpack::lzss_decoder decoder;
        decoder.decode(begin(data), end(data), debug_lzss_decoder_receiver(std::cout, options.flags));
    }
    catch (const ifstream::failure&)
    {
        // Depending on the library, exceptions thrown by ifstream may have rather useless error messages,
        // so we catch exceptions here and try our luck with errno and std::system_error instead.
        auto error = errno;
        throw std::system_error(error, std::generic_category(), "could not analyze " + options.input_file);
    }
}

}

int main(int argc, char* argv[])
{
    try
    {
        auto options = parse_command_line(argc, argv);
        analyze_file(options);
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << argv[0] << ": error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

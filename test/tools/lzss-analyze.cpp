// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <string>
#include <stdexcept>

import agbpack;

namespace
{

using byte_vector = std::vector<unsigned char>;
using size_t = std::size_t;
using string = std::string;
using std::format;

struct command_line_options final
{
    string input_file;
};

class debug_lzss_decoder_receiver final
{
public:
    explicit debug_lzss_decoder_receiver(std::ostream& os) : m_os(os) {}

    void tags(unsigned char tags)
    {
        m_os << format("{:#06x} T {:#010b} ({:#04x})\n", file_position(), tags, tags);
    }

    void literal(unsigned char c)
    {
        m_os << format("{:#06x} L '{}'\n", file_position(), escape_character(c));
        m_uncompressed_data.push_back(c);
    }

    void reference(size_t length, size_t offset)
    {
        m_os << format("{:#06x} R {:2} {:4} '", file_position(), length, offset);

        while (length--)
        {
            unsigned char c = m_uncompressed_data[m_uncompressed_data.size() - offset];
            m_os << escape_character(c);
            m_uncompressed_data.push_back(c);
        }

        m_os << "'\n";
    }

private:
    size_t file_position() const { return m_uncompressed_data.size(); }

    string escape_character(unsigned char byte)
    {
        auto c = char(byte);

        if ((c == '\\') || !std::isprint(c))
        {
            return format("\\x{:02x}", c);
        }

        return string(1, c);
    }

    std::ostream& m_os;
    byte_vector m_uncompressed_data;
};


command_line_options parse_command_line(int argc, char* argv[])
{
    if (argc != 2)
    {
        throw std::runtime_error("wrong number of arguments");
    }

    return command_line_options{argv[1]};
}

std::ifstream open_binary_file(const std::string& path)
{
    // The exceptions thrown by ifstream when opening fails have rather useless error messages.
    // For instance, MSVC throws an exception with the following message: 'ios_base::failbit set: iostream stream error'.
    // So we don't use stream exceptions and try our luck with errno and std::system_error.
    std::ifstream file(path, std::ios_base::binary);

    if (!file)
    {
        auto error = errno;
        throw std::system_error(error, std::generic_category(), "could not open " + path);
    }

    // This is required to correctly read binary files using some APIs, e.g. std::istream_iterator.
    file.unsetf(std::ios::skipws);

    return file;
}

// TODO: turn around exception handling here: enable exceptions before opening the file, and handle them only once, top level, right here.
void analyze_file(const string& filename)
{
    auto filestream = open_binary_file(filename);
    agbpack::lzss_decoder decoder;
    decoder.decode(
        std::istream_iterator<unsigned char>(filestream),
        std::istream_iterator<unsigned char>(),
        debug_lzss_decoder_receiver(std::cout));
}

}

int main(int argc, char* argv[])
{
    try
    {
        auto options = parse_command_line(argc, argv);
        analyze_file(options.input_file);
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << argv[0] << ": error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

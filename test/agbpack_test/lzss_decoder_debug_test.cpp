// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <cctype>
#include <cstddef>
#include <format>
#include <iostream>
#include <sstream>
#include <vector>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

using byte_vector = std::vector<unsigned char>;
using size_t = std::size_t;
using string = std::string;
using std::format;

namespace
{

class debug_lzss_decoder_reciver final
{
public:
    explicit debug_lzss_decoder_reciver(std::ostream& os) : m_os(os) {}

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


TEST_CASE_METHOD(test_data_fixture, "lzss_decoder_debug_test")
{
    agbpack::lzss_decoder decoder;
    agbpack::lzss_encoder encoder;
    set_test_data_directory("lzss_encoder");

    SECTION("Test whether generating LZSS decoder debug output works")
    {
        const byte_vector decoded_data{'a', 'a', 'a', 'a', 'b', '\\', '\n'};
        const auto encoded_data = encode_vector(encoder, decoded_data);
        std::stringstream debug_output_stream;

        decoder.decode(begin(encoded_data), end(encoded_data), debug_lzss_decoder_reciver(debug_output_stream));

        CHECK(debug_output_stream.str() ==
            "0x0000 T 0b01000000 (0x40)\n"
            "0x0000 L 'a'\n"
            "0x0001 R  3    1 'aaa'\n"
            "0x0004 L 'b'\n"
            "0x0005 L '\\x5c'\n"
            "0x0006 L '\\x0a'\n");
    }

    /*
    SECTION("Debug output")
    {
        const auto decoded_data = read_decoded_file("lzss.good.delta.cppm");
        const auto encoded_data = encode_vector(encoder, decoded_data);

        decoder.decode(begin(encoded_data), end(encoded_data), debug_lzss_decoder_reciver(std::cout));
    }
    */
}

}

}

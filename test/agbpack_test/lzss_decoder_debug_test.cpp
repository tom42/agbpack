// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <format>
#include <iostream>
#include <vector>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

using size_t = std::size_t;

namespace
{

class debug_lzss_decoder_reciver final
{
public:
    explicit debug_lzss_decoder_reciver(std::ostream& os) : m_os(os) {}

    void tags(unsigned char tags)
    {
        m_os << std::format("{:#06x} T {:#010b} ({:#04x})\n", file_position(), tags, tags);
    }

    void literal(unsigned char c)
    {
        m_os << std::format("{:#06x} L '{}'\n", file_position(), char(c));
        m_uncompressed_data.push_back(c);
    }

    void reference(size_t length, size_t offset)
    {
        m_os << std::format("{:#06x} R {:2} {:4} '", file_position(), length, offset);

        while (length--)
        {
            unsigned char c = m_uncompressed_data[m_uncompressed_data.size() - offset];
            m_os << c;
            m_uncompressed_data.push_back(c);
        }

        m_os << "'\n";
    }

private:
    size_t file_position() const { return m_uncompressed_data.size(); }

    std::ostream& m_os;
    std::vector<unsigned char> m_uncompressed_data;
};


TEST_CASE_METHOD(test_data_fixture, "lzss_decoder_debug_test")
{
    agbpack::lzss_decoder decoder;
    set_test_data_directory("lzss_decoder");

    // TODO: this test is only here to develop debug output. We're probably going to delete it again. Or are we?
    //       => Well we're going to move it into a separate file, cerainly
    //       => We can then have a simple test that produces no output and just ensures things keep building
    //       => When needed we can uncomment a test that produces output
    //       => Finally we're probably more interested in the test data from the encoder test (and, incidentally, we are debugging the encoder with this, not the decoder)
    SECTION("Debug output")
    {
        const auto encoded_data = read_encoded_file("lzss.good.literals-and-references.txt");
        decoder.decode(begin(encoded_data), end(encoded_data), debug_lzss_decoder_reciver(std::cout));
    }
}

}

}

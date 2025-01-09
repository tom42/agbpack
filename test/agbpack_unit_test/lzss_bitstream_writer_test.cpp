// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <vector>

import agbpack;

namespace agbpack_unit_test
{

using agbpack::lzss_bitstream_writer;
using bitstream = std::vector<unsigned char>;

TEST_CASE("lzss_bitstream_writer_test")
{
    bitstream actual_bitstream;
    lzss_bitstream_writer writer(actual_bitstream);

    SECTION("Write literal bytes")
    {
        writer.write_literal(0x11);
        writer.write_literal(0x22);
        writer.write_literal(0x33);
        writer.write_literal(0x44);
        writer.write_literal(0x55);
        writer.write_literal(0x66);
        writer.write_literal(0x77);
        writer.write_literal(0x88);
        writer.write_literal(0x99);

        bitstream expected_bitstream =
        {
            0x00,
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
            0x00,
            0x99
        };

        CHECK(actual_bitstream == expected_bitstream);
    }

    SECTION("Write references")
    {
        // TODO: actually pass length and offset and check it works
        writer.write_reference(0, 0);
    }
}

}

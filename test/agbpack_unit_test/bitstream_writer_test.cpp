// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <iterator>
#include <vector>

import agbpack;

namespace agbpack_unit_test
{

using byte_vector = std::vector<unsigned char>;

TEST_CASE("bitstream_writer_test")
{
    byte_vector output;
    agbpack::unbounded_byte_writer byte_writer(back_inserter(output));
    agbpack::bitstream_writer bitstream_writer(byte_writer);

    SECTION("Write codes of length 1 without overflow of the bit buffer")
    {
        bitstream_writer.write_code(1, 1);
        bitstream_writer.write_code(0, 1);
        bitstream_writer.write_code(1, 1);
        bitstream_writer.write_code(0, 1);
        bitstream_writer.write_code(0, 1);
        bitstream_writer.write_code(1, 1);
        bitstream_writer.flush();

        CHECK(output == byte_vector{ 0, 0, 0, 0b10100100 });
    }

    SECTION("Write codes of length greater than 1 without overflow of the bit buffer")
    {
        bitstream_writer.write_code(0b10, 2);
        bitstream_writer.write_code(0b011, 3);
        bitstream_writer.write_code(0b010111, 6);
        bitstream_writer.flush();

        CHECK(output == byte_vector{ 0, 0, 0b11100000, 0b10011010 });
    }

    SECTION("flush when no data has been written")
    {
        bitstream_writer.flush();

        CHECK(output == byte_vector{});
    }

    SECTION("Repeated calls to flush should have no efect")
    {
        bitstream_writer.write_code(1, 1);

        bitstream_writer.flush();
        bitstream_writer.flush();

        CHECK(output == byte_vector{ 0, 0, 0, 0b10000000});
    }
}

}

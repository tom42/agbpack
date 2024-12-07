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

    SECTION("flush when no data has been written")
    {
        bitstream_writer.flush();

        CHECK(output == byte_vector{});
    }
}

}

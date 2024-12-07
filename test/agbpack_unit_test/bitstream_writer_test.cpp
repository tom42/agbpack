// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <iterator>
#include <vector>

import agbpack;

namespace agbpack_unit_test
{

TEST_CASE("bitstream_writer_test")
{
    std::vector<unsigned char> output;
    agbpack::unbounded_byte_writer x(back_inserter(output)); // TODO: make temporary
    agbpack::bitstream_writer y(x); // TODO: better name

    x.write8(0);

    // TODO: implement and test new bitstream_writer
    CHECK(false);
}

}

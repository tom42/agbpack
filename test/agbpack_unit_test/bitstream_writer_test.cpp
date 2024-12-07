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
    agbpack::unbounded_byte_writer byte_writer(back_inserter(output));
    agbpack::bitstream_writer bitstream_writer(byte_writer);

    // TODO: implement and test new bitstream_writer
    CHECK(false);
}

}

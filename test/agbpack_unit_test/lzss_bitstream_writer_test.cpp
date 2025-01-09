// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <vector>

import agbpack;

namespace agbpack_unit_test
{

using agbpack::lzss_bitstream_writer;

TEST_CASE("lzss_bitstream_writer_test")
{
    std::vector<unsigned char> bitstream;
    lzss_bitstream_writer writer(bitstream);
}

}

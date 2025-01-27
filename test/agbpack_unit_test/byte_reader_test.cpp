// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <utility>
#include <vector>

import agbpack;
import agbpack_unit_testkit;

namespace agbpack_unit_test
{

using byte_reader = agbpack::byte_reader;
using byte_vector = std::vector<unsigned char>;
using pair = std::pair;

// TODO: write test for existing functionality of byte_reader
// TODO: add non-throwing version of read8
// TODO: add non-throwing version of read16 (or whatever API delta encoder uses)
TEST_CASE("byte_reader_test")
{
    SECTION("State after creation")
    {
        const auto [input, expected_eof] = GENERATE(
            pair(byte_vector{}, true),
            pair(byte_vector{ 0x01, 0x02 }, false));
        byte_reader reader(begin(input), end(input));

        CHECK(reader.eof() == expected_eof);
        CHECK(reader.nbytes_read() == 0);
    }

    SECTION("Read data until EOF is reached")
    {
        byte_vector input{ 0x01, 0x02 };
        byte_reader reader(begin(input), end(input));

        // TODO: read data. Check
        //       * read byte
        //       * eof
        //       * nbytes_read
    }
}

}

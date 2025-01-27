// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <vector>

import agbpack;
import agbpack_unit_testkit;

namespace agbpack_unit_test
{

// TODO: write test for existing functionality of byte_reader
// TODO: add non-throwing version of read8
// TODO: add non-throwing version of read16 (or whatever API delta encoder uses)
TEST_CASE("byte_reader_test")
{
    SECTION("Empty input")
    {
        // TODO: allow easy creation of readers
        // TODO: make this test case: state after creation:
        //       * Can deal with empty input
        //       * Can deal with non-empty input
        std::vector<unsigned char> input;
        agbpack::byte_reader reader(begin(input), end(input));

        CHECK(reader.eof() == true);
        CHECK(reader.nbytes_read() == 0);
    }
}

}

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

using agbpack::byte_reader;
using agbpack::decode_exception;
using byte_vector = std::vector<unsigned char>;
using std::pair;

TEST_CASE("byte_reader_test")
{
    SECTION("State after creation")
    {
        const auto [input, expected_eof] = GENERATE(
            pair(byte_vector{}, true),
            pair(byte_vector{ 0, 0 }, false));
        byte_reader reader(begin(input), end(input));

        CHECK(reader.eof() == expected_eof);
        CHECK(reader.nbytes_read() == 0);
    }

    SECTION("Read data until EOF is reached")
    {
        byte_vector input{ 11, 22 };
        byte_reader reader(begin(input), end(input));

        CHECK(reader.read8() == 11);
        CHECK(reader.nbytes_read() == 1);
        CHECK(reader.eof() == false);

        CHECK(reader.read8() == 22);
        CHECK(reader.nbytes_read() == 2);
        CHECK(reader.eof() == true);
    }

    SECTION("read8 throws if it's called when EOF has been reached")
    {
        byte_vector input{ 0 };
        byte_reader reader(begin(input), end(input));

        reader.read8();

        CHECK(reader.eof() == true);
        CHECK_THROWS_AS(reader.read8(), decode_exception);
    }

    SECTION("peek8 reads current byte without advancing")
    {
        byte_vector input{ 11, 22 };
        byte_reader reader(begin(input), end(input));

        CHECK(reader.peek8() == 11);
        CHECK(reader.read8() == 11);

        CHECK(reader.peek8() == 22);
        CHECK(reader.read8() == 22);

        CHECK(reader.nbytes_read() == 2);
    }
}

}

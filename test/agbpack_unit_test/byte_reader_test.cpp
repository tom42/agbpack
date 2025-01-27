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

// TODO: add non-throwing version of read16 (or whatever API delta encoder uses)
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

    SECTION("Try reading data until EOF is reached")
    {
        byte_vector input{ 11, 22 };
        byte_reader reader(begin(input), end(input));

        CHECK(reader.try_read8() == 11);
        CHECK(reader.nbytes_read() == 1);
        CHECK(reader.eof() == false);

        CHECK(reader.try_read8() == 22);
        CHECK(reader.nbytes_read() == 2);
        CHECK(reader.eof() == true);
    }

    SECTION("try_read8 returns empty optional if it's called when EOF has been reached")
    {
        byte_vector input{ 0 };
        byte_reader reader(begin(input), end(input));

        reader.read8();

        CHECK(reader.eof() == true);
        CHECK(reader.try_read8().has_value() == false);
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

    SECTION("read16")
    {
        byte_vector input{ 0x34, 0x12, 0x78, 0x56 };
        byte_reader reader(begin(input), end(input));

        CHECK(agbpack::read16(reader) == 0x1234);
        CHECK(agbpack::read16(reader) == 0x5678);
    }
}

}

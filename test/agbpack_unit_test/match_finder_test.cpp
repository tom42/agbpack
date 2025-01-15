// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <format>
#include <string>

import agbpack;
import agbpack_unit_test;

namespace agbpack_unit_test
{

using agbpack::match;
using agbpack::match_finder;

namespace
{

match find_match(const std::string& data, std::size_t current_position, std::size_t minimum_match_offset)
{
    std::vector<unsigned char> v(data.begin(), data.end());
    match_finder match_finder(v, minimum_match_offset);
    return match_finder.find_match(current_position);
}

match find_match_wram(const std::string& data, std::size_t current_position)
{
    return find_match(data, current_position, 0);
}

match find_match_vram(const std::string& data, std::size_t current_position)
{
    return find_match(data, current_position, 1);
}

}

TEST_CASE("match_finder_test")
{
    SECTION("Empty input")
    {
        CHECK(find_match_wram("", 0) == match(0, 0));
        CHECK(find_match_wram("", 1) == match(0, 0));
    }

    SECTION("Literal bytes only")
    {
        CHECK(find_match_wram("a", 0) == match(0, 0));
        CHECK(find_match_wram("a", 1) == match(0, 0));
        CHECK(find_match_wram("ab", 0) == match(0, 0));
        CHECK(find_match_wram("ab", 1) == match(0, 0));
        CHECK(find_match_wram("ab", 2) == match(0, 0));
        CHECK(find_match_wram("abc", 0) == match(0, 0));
        CHECK(find_match_wram("abc", 1) == match(0, 0));
        CHECK(find_match_wram("abc", 2) == match(0, 0));
        CHECK(find_match_wram("abc", 3) == match(0, 0));
    }

    SECTION("Reference of length 1")
    {
        // First 'a' is no match, second one is a match of length 1
        CHECK(find_match_wram("aa", 0) == match(0, 0));
        CHECK(find_match_wram("aa", 1) == match(1, 1));

        // Same, but a different character rather than EOF terminates the match
        CHECK(find_match_wram("aab", 0) == match(0, 0));
        CHECK(find_match_wram("aab", 1) == match(1, 1));
    }

    SECTION("Reference of length 2")
    {
        // First 'a' is no match, second one is a match of length 2, third one length 1 again
        CHECK(find_match_wram("aaa", 0) == match(0, 0));
        CHECK(find_match_wram("aaa", 1) == match(2, 1));
        CHECK(find_match_wram("aaa", 2) == match(1, 2));

        // Same, but a different character rather than EOF terminates the match
        CHECK(find_match_wram("aaab", 0) == match(0, 0));
        CHECK(find_match_wram("aaab", 1) == match(2, 1));
        CHECK(find_match_wram("aaab", 2) == match(1, 2));
    }

    SECTION("Reference of length 18 that overlaps with lookahead buffer")
    {
        CHECK(find_match_wram("aaaaaaaaaaaaaaaaaaa", 0) == match(0, 0));
        CHECK(find_match_wram("aaaaaaaaaaaaaaaaaaa", 1) == match(18, 1));
        CHECK(find_match_wram("aaaaaaaaaaaaaaaaaaa", 2) == match(17, 2));
    }

    SECTION("If there is more than one match the longer one is returned")
    {
        // For current position 5 there are two matches:
        // * A short one with length=3 and offset=5
        // * A long one with length=18 and offset=1
        // We want the longer one
        CHECK(find_match_wram("aaabaaaaaaaaaaaaaaaaaaa", 5) == match(18, 1));
    }

    SECTION("VRAM safe encoding does not return matches with offset=1")
    {
        auto input = "aaaaaaaa";
        CHECK(find_match_wram(input, 1) == match(7, 1));
        CHECK(find_match_vram(input, 1) == match(0, 0));
        CHECK(find_match_vram(input, 2) == match(6, 2));
    }
}

}

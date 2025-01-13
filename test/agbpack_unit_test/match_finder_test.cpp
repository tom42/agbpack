// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <format>
#include <string>

import agbpack;

namespace Catch
{

// TODO: are we risking ODR violations here?
//       See https://brevzin.github.io/c++/2023/01/19/debug-fmt-catch/
template <>
struct StringMaker<agbpack::match>
{
    static std::string convert(const agbpack::match& m)
    {
        return std::format("(length={}, offset={})", m.length(), m.offset());
    }
};

}

namespace agbpack_unit_test
{

using agbpack::match;
using agbpack::match_finder;

namespace
{

// TODO: factor out things, so that find_match and find_match_vram don't duplicate that much code
match find_match_wram(const std::string& data, std::size_t current_position)
{
    std::vector<unsigned char> v(data.begin(), data.end());
    match_finder match_finder(v, 0);
    return match_finder.find_match(current_position);
}

match find_match_vram(const std::string& data, std::size_t current_position)
{
    std::vector<unsigned char> v(data.begin(), data.end());
    match_finder match_finder(v, 0); // TODO: somehow tell match_finder about its minimum offset
    return match_finder.find_match(current_position);
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
        // TODO: do something here
        auto input = "aaaaaaaa";
        CHECK(find_match_wram(input, 1) == match(7, 1));
    }
}

}

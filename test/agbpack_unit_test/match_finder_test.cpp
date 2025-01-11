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

match find_match(const std::string& data, std::size_t current_position)
{
    std::vector<unsigned char> v(data.begin(), data.end());
    match_finder match_finder(v);
    return match_finder.find_match(current_position);
}

}

TEST_CASE("match_finder_test")
{
    SECTION("Empty input")
    {
        CHECK(find_match("", 0) == match(0, 0));
        CHECK(find_match("", 1) == match(0, 0));
    }

    SECTION("Literal bytes only")
    {
        CHECK(find_match("a", 0) == match(0, 0));
        CHECK(find_match("a", 1) == match(0, 0));
        CHECK(find_match("ab", 0) == match(0, 0));
        CHECK(find_match("ab", 1) == match(0, 0));
        CHECK(find_match("ab", 2) == match(0, 0));
        CHECK(find_match("abc", 0) == match(0, 0));
        CHECK(find_match("abc", 1) == match(0, 0));
        CHECK(find_match("abc", 2) == match(0, 0));
        CHECK(find_match("abc", 3) == match(0, 0));
    }

    SECTION("Reference of length 1")
    {
        // First 'a' is no match, second one is a match of length 1
        CHECK(find_match("aa", 0) == match(0, 0));
        CHECK(find_match("aa", 1) == match(1, 1));

        // Same, but a different character rather than EOF terminates the match
        CHECK(find_match("aab", 0) == match(0, 0));
        CHECK(find_match("aab", 1) == match(1, 1));
    }

    SECTION("Reference of length 2")
    {
        // First 'a' is no match, second one is a match of length 2, third one length 1 again
        CHECK(find_match("aaa", 0) == match(0, 0));
        CHECK(find_match("aaa", 1) == match(2, 1));
        CHECK(find_match("aaa", 2) == match(1, 2));

        // Same, but a different character rather than EOF terminates the match
        CHECK(find_match("aaab", 0) == match(0, 0));
        CHECK(find_match("aaab", 1) == match(2, 1));
        CHECK(find_match("aaab", 2) == match(1, 2));
    }

    SECTION("Reference of length 18 that overlaps with lookahead buffer")
    {
        CHECK(find_match("aaaaaaaaaaaaaaaaaaa", 0) == match(0, 0));
        CHECK(find_match("aaaaaaaaaaaaaaaaaaa", 1) == match(18, 1));
        CHECK(find_match("aaaaaaaaaaaaaaaaaaa", 2) == match(17, 2));
    }
}

}

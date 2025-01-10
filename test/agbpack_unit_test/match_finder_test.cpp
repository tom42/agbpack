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
    // TODO: do we also check with input length and current offset > 0? What do we return for match position in THAT case?
    //       => Well sure we want to do that, but if input length is nonzero then it does not go into this section
    SECTION("Empty input")
    {
        CHECK(find_match("", 0) == match(0, 0));
        CHECK(find_match("", 1) == match(0, 0));
    }

    SECTION("Literal bytes only") // TODO: section name?
    {
        // TODO: find a way to say e.g. match::none()
        CHECK(find_match("a", 0) == match(0, 0));  // TODO: do we also test with input position = 1?
        CHECK(find_match("a", 1) == match(0, 0));  // TODO: do we also test with input position = 1?
        CHECK(find_match("ab", 0) == match(0, 0));
        CHECK(find_match("ab", 1) == match(0, 0));
        CHECK(find_match("ab", 2) == match(0, 0));
    }
}

}

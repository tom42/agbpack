// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

import agbpack;
import agbpack_unit_testkit;

namespace agbpack_unit_test
{

using byte_vector = std::vector<unsigned char>;
using m = agbpack::match;
using mv = std::vector<agbpack::match>;

namespace
{

auto call_find_longest_matches(const std::string& s)
{
    return agbpack::find_longest_matches(byte_vector(begin(s), end(s)), false);
}

}

TEST_CASE("find_longest_matches_test")
{
    CHECK(call_find_longest_matches("")      == mv{});
    CHECK(call_find_longest_matches("a")     == mv{ m(1, 0) });
    CHECK(call_find_longest_matches("aa")    == mv{ m(1, 0), m(1, 0) });
    CHECK(call_find_longest_matches("ab")    == mv{ m(1, 0), m(1, 0) });
    CHECK(call_find_longest_matches("aaa")   == mv{ m(1, 0), m(1, 0), m(1, 0) });
    CHECK(call_find_longest_matches("aaaa")  == mv{ m(1, 0), m(3, 1), m(1, 0), m(1, 0) });
    CHECK(call_find_longest_matches("aaaaa") == mv{ m(1, 0), m(4, 1), m(3, 2), m(1, 0), m(1, 0) });
    CHECK(call_find_longest_matches("ababa") == mv{ m(1, 0), m(1, 0), m(3, 2), m(1, 0), m(1, 0) });
}

}

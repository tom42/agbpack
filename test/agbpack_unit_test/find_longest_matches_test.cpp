// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

import agbpack;
import agbpack_unit_testkit;

namespace agbpack_unit_test
{

using m = agbpack::match;
using byte_vector = std::vector<unsigned char>;
using match_vector = std::vector<agbpack::match>;

namespace
{

auto call_find_longest_matches(const std::string& s)
{
    return agbpack::find_longest_matches(byte_vector(begin(s), end(s)));
}

}

TEST_CASE("find_longest_matches_test")
{
    CHECK(call_find_longest_matches("")      == match_vector{});
    CHECK(call_find_longest_matches("a")     == match_vector{ m(1, 0) });
    CHECK(call_find_longest_matches("aa")    == match_vector{ m(1, 0), m(1, 0) });
    CHECK(call_find_longest_matches("ab")    == match_vector{ m(1, 0), m(1, 0) });
    CHECK(call_find_longest_matches("aaa")   == match_vector{ m(1, 0), m(1, 0), m(1, 0) }); // TODO: how does the greedy encoder encode this, then? Three literals?
    CHECK(call_find_longest_matches("aaaa")  == match_vector{ m(1, 0), m(3, 1), m(1, 0), m(1, 0) });
    CHECK(call_find_longest_matches("aaaaa") == match_vector{ m(1, 0), m(4, 1), m(3, 2), m(1, 0), m(1, 0) }); // TODO: better to use the first longest match or the last one? (for entropy coding?)
    CHECK(call_find_longest_matches("ababa") == match_vector{ m(1, 0), m(1, 0), m(3, 2), m(1, 0), m(1, 0) });
}

}

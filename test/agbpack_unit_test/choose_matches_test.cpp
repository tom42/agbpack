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

auto call_choose_matches(const std::string& s)
{
    const auto ml = agbpack::find_longest_matches(byte_vector(begin(s), end(s)));
    return agbpack::choose_matches(ml);
}

}

TEST_CASE("choose_matches_test")
{
    // TODO: do not forget: we are still quite unsure whether our code is correct in all cases
    // TODO: arbitrary input
    // TODO: can we find some sort of input where the optimal parsing actually does help
    CHECK(call_choose_matches("")       == mv{});
    CHECK(call_choose_matches("a")      == mv{ m(1, 0) });
    CHECK(call_choose_matches("ab")     == mv{ m(1, 0), m(1, 0) });
    CHECK(call_choose_matches("aaa")    == mv{ m(1, 0), m(1, 0), m(1, 0) });
    CHECK(call_choose_matches("aaaa")   == mv{ m(1, 0), m(3, 1), m(1, 0), m(1, 0) });
    CHECK(call_choose_matches("abcabc") == mv{ m(1, 0), m(1, 0), m(1, 0), m(3, 3), m(1, 0), m(1, 0) });
}

}

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
    agbpack::find_longest_matches(byte_vector(begin(s), end(s)));
    return agbpack::choose_matches({});
}

}

TEST_CASE("choose_matches_test")
{
    // TODO: input of length 1
    // TODO: input of length 2
    // TODO: arbitrary input
    // TODO: can we find some sort of input where the optimal parsing actually does help
    CHECK(call_choose_matches("") == mv{});
}

}

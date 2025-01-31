// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import agbpack;
import agbpack_unit_testkit;

namespace agbpack_unit_test
{

namespace
{

auto call_choose_matches()
{
    // TODO: have a way to supply input. Do we supply matches, or a string as input? Does it matter? (Do we simply check degenerate cases like empty input here?)
    return agbpack::choose_matches({});
}

}

TEST_CASE("choose_matches_test")
{
    // TODO: input of length 0
    // TODO: input of length 1
    // TODO: input of length 2
    // TODO: arbitrary input
    // TODO: can we find some sort of input where the optimal parsing actually does help
    call_choose_matches(); // TODO: assert something here
}

}

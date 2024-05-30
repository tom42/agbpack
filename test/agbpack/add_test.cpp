// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

// TODO: this file is going to be deleted again
import agbpack;
#include <catch2/catch_test_macros.hpp>

TEST_CASE("add_test")
{
    REQUIRE(agbpack::add(2, 3) == 5);
    REQUIRE(agbpack::add(3, 5) == 8);
}

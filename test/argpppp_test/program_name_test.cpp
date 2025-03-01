// SPDX-FileCopyrightText: 2025 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import argpppp;

namespace argpppp_test
{

TEST_CASE("program_name_test")
{
    CHECK(argpppp::program_name("foo/bar/baz.exe") == "baz");
}

}

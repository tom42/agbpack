// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import agbpack;

namespace agbpack_unit_test
{

using agbpack::tree_node;

TEST_CASE("tree_node_test")
{
    SECTION("make_leaf")
    {
        auto leaf = tree_node::make_leaf('A', 42);

        // TODO: check value
        // TODO: check frequency
        // TODO: there should be no children
        CHECK(false);
    }
}

}

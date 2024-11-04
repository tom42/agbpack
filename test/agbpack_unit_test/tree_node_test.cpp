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

        CHECK(leaf->child(0) == nullptr);
        CHECK(leaf->child(1) == nullptr);
        CHECK(leaf->value() == 'A');
        CHECK(leaf->frequency() == 42);
    }
}

}

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
        auto leaf_node = tree_node::make_leaf('A', 42);

        CHECK(leaf_node->is_parent() == false);
        CHECK(leaf_node->child(0) == nullptr);
        CHECK(leaf_node->child(1) == nullptr);
        CHECK(leaf_node->value() == 'A');
        CHECK(leaf_node->frequency() == 42);
    }

    SECTION("make_internal")
    {
        auto child0 = tree_node::make_leaf('B', 43);
        auto child1 = tree_node::make_leaf('C', 44);
        auto internal_node = tree_node::make_internal(child0, child1);

        CHECK(internal_node->is_parent() == true);
        CHECK(internal_node->child(0) == child0);
        CHECK(internal_node->child(1) == child1);
        CHECK(internal_node->value() == 0);
        CHECK(internal_node->frequency() == 87);
    }
}

}

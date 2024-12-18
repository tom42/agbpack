// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import agbpack;

namespace agbpack_unit_test
{

using agbpack::huffman_tree_node;

TEST_CASE("tree_node_test")
{
    SECTION("make_leaf")
    {
        auto leaf_node = huffman_tree_node::make_leaf('A', 42);

        CHECK(leaf_node->is_internal() == false);
        CHECK(leaf_node->child(0) == nullptr);
        CHECK(leaf_node->child(1) == nullptr);
        CHECK(leaf_node->sym() == 'A');
        CHECK(leaf_node->frequency() == 42);
    }

    SECTION("make_internal")
    {
        auto child0 = huffman_tree_node::make_leaf('B', 43);
        auto child1 = huffman_tree_node::make_leaf('C', 44);
        auto internal_node = huffman_tree_node::make_internal(std::move(child0), std::move(child1));

        CHECK(internal_node->is_internal() == true);
        CHECK(internal_node->child(0)->sym() == 'B');
        CHECK(internal_node->child(1)->sym() == 'C');
        CHECK(internal_node->sym() == 0);
        CHECK(internal_node->frequency() == 87);
    }

    SECTION("num_nodes and num_leaves, leaf node")
    {
        auto leaf_node = huffman_tree_node::make_leaf(0, 0);

        CHECK(leaf_node->num_nodes() == 1);
        CHECK(leaf_node->num_leaves() == 1);
    }

    SECTION("num_nodes and num_leaves, internal node")
    {
        auto child0 = huffman_tree_node::make_leaf(0, 0);
        auto child1 = huffman_tree_node::make_leaf(0, 0);
        auto child2 = huffman_tree_node::make_leaf(0, 0);
        auto internal_node0 = huffman_tree_node::make_internal(std::move(child0), std::move(child1));
        auto root = huffman_tree_node::make_internal(std::move(internal_node0), std::move(child2));

        CHECK(root->child(0)->num_nodes() == 3);
        CHECK(root->num_nodes() == 5);

        CHECK(root->child(0)->num_leaves() == 2);
        CHECK(root->num_leaves() == 3);
    }
}

}

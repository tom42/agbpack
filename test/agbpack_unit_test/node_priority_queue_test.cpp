// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import agbpack;
import agbpack_unit_testkit;

namespace agbpack_unit_test
{

using agbpack::huffman_tree_node;
using agbpack::node_priority_queue;

TEST_CASE("node_priority_queue_test")
{
    node_priority_queue queue;
    queue.reserve(3);

    SECTION("Nodes are popped in correct order")
    {
        queue.push(huffman_tree_node::make_leaf('a', 3));
        queue.push(huffman_tree_node::make_leaf('b', 1));
        queue.push(huffman_tree_node::make_leaf('c', 2));

        CHECK(queue.pop()->frequency() == 1);
        CHECK(queue.pop()->frequency() == 2);
        CHECK(queue.pop()->frequency() == 3);
    }

    SECTION("Size")
    {
        CHECK(queue.size() == 0);

        queue.push(huffman_tree_node::make_leaf(0, 0));
        CHECK(queue.size() == 1);

        queue.push(huffman_tree_node::make_leaf(0, 0));
        CHECK(queue.size() == 2);

        queue.pop();
        CHECK(queue.size() == 1);
    }
}

}

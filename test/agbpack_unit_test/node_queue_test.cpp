// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import agbpack;

namespace agbpack_unit_test
{

using agbpack::Node;
using agbpack::node_queue;

TEST_CASE("node_queue_test")
{
    node_queue queue;

    queue.reserve(3);

    queue.push(Node::make_leaf('a', 3));
    queue.push(Node::make_leaf('b', 1));
    queue.push(Node::make_leaf('c', 2));

    CHECK(queue.pop()->frequency() == 1);
    CHECK(queue.pop()->frequency() == 2);
    CHECK(queue.pop()->frequency() == 3);
}

}

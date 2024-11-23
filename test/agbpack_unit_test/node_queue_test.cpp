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

    queue.push(nullptr); // TODO: not null
    queue.pop();
}

}

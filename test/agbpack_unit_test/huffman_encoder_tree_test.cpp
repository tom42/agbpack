// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import agbpack;

namespace agbpack_unit_test
{

using agbpack::frequency_table;
using agbpack::huffman_encoder_tree;

// TODO: tests
//       * No symbols
//       * One symbol
//       * Two symbols
//       * Tree with 256 symbols with same frequency
//       * Maximum depth tree
//       * Maximum handleable code length exceeded
TEST_CASE("huffman_encoder_tree_test")
{
    SECTION("No symbols")
    {
        frequency_table frequencies(8);
        huffman_encoder_tree tree(8, frequencies);

        // TODO: assert serialized tree. This one should be pretty simple: tree size byte, root node, two bogus child nodes
    }
}

}

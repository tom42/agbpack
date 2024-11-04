// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

import agbpack;

using agbpack::huffman_encoder_tree;

namespace agbpack_unit_test
{

// TODO: tests
//       * No symbols
//       * One symbol
//       * Two symbols
//       * Tree with 256 symbols with same frequency
//       * Maximum depth tree
//       * Maximum handleable code length exceeded
TEST_CASE("huffman_encoder_tree_test")
{
}

}

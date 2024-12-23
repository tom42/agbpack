// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <vector>

import agbpack;

namespace agbpack_unit_test
{

using agbpack::huffman_decoder_tree;

TEST_CASE("huffman_decoder_tree_test")
{
    SECTION("create_code_table encounters garbage in unused bits of symbol")
    {
        // TODO: add test: create_decode_tree when there is garbage in a node
        const auto symbol_size = 4;
        std::vector<unsigned char> serialized_tree{ 0x00, 0x00, 0x00, 0x00}; // TODO: real content

        // TODO: read/construct tree
        huffman_decoder_tree tree(symbol_size, );

        // TODO: call create_code_table, should fail because of garbage in high bits of tree
        FAIL();
    }
}

}

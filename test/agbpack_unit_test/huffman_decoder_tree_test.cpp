// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <vector>

import agbpack;

namespace agbpack_unit_test
{

using agbpack::byte_reader;
using agbpack::huffman_decoder_tree;
using std::vector;

namespace
{

auto create_huffman_decoder_tree(unsigned int symbol_size, const vector<unsigned char>& serialized_tree)
{
    byte_reader byte_reader(begin(serialized_tree), end(serialized_tree));
    return huffman_decoder_tree(symbol_size, byte_reader);
}

}

TEST_CASE("huffman_decoder_tree_test")
{
    SECTION("create_code_table encounters garbage in unused bits of symbol")
    {
        const auto symbol_size = 4;
        auto tree = create_huffman_decoder_tree(symbol_size, { 0x00, 0x00, 0x00, 0x00}); // TODO: real content

        // TODO: call create_code_table, should fail because of garbage in high bits of tree
        // TODO: this already fails, but probably for some other reason. We should really have better exception messages...
        tree.create_code_table();
        FAIL();
    }
}

}

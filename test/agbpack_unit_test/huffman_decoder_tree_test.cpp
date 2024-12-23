// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

namespace agbpack_unit_test
{

TEST_CASE("huffman_decoder_tree_test")
{
    SECTION("create_code_table encounters garbage in unused bits of symbol")
    {
        // TODO: add test: create_decode_tree when there is garbage in a node
        FAIL();
    }
}

}

// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <vector>

import agbpack;
import agbpack_unit_testkit;

namespace agbpack_unit_test
{

using agbpack::byte_reader;
using agbpack::decode_exception;
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
        auto tree = create_huffman_decoder_tree(symbol_size, { 0x01, 0xc0, 0x11, 0x02});

        CHECK_THROWS_MATCHES(
            tree.create_code_table(),
            decode_exception,
            Catch::Matchers::Message("encoded data is corrupt: huffman tree contains invalid symbol"));
    }
}

}

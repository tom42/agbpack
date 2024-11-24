// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <ostream>

import agbpack;
import agbpack_unit_test.utility;

namespace
{

bool operator==(const agbpack::code_table_entry& /*a*/, const agbpack::code_table_entry& /*b*/)
{
    // TODO: compare all components (symbol, code, code length)
    return false;
}

std::ostream& operator<<(std::ostream& os, const agbpack::code_table_entry& /*e*/)
{
    // TODO: output all components (symbol, code, code length)
    os << "x";
    return os;
}

}

namespace agbpack_unit_test
{

using agbpack::code_table_entry;
using agbpack::frequency_table;
using agbpack::huffman_encoder_tree;
using agbpack::max_code_length;
using agbpack::symbol;

TEST_CASE("huffman_encoder_tree_test")
{
    constexpr auto symbol_size = 8;
    frequency_table frequencies(symbol_size);

    SECTION("Create code table with maximum code length")
    {
        // TODO: factor this out, we need it at least once more
        auto s = lucas_sequence(max_code_length + 1);
        for (symbol i = 0; i < s.size(); ++i)
        {
            frequencies.set_frequency(i, s[i]);
        }

        huffman_encoder_tree tree(symbol_size, frequencies);

        auto code_table = tree.create_code_table();

        // TODO: check all codes and lengths. Problem: how?
        CHECK(code_table[0] == code_table_entry(0, 32, 0b11111111111111111111111111111010));
        CHECK(code_table[1] == code_table_entry(1, 32, 0b11111111111111111111111111111011));
    }

    SECTION("Maximum code length exceeded")
    {
        // TODO: create_code_table: maximum code length exceeded
    }
}

}

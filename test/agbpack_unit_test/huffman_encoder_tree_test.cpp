// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <format>
#include <ostream>
#include <string>

import agbpack;
import agbpack_unit_test.utility;

namespace Catch
{

template <>
struct StringMaker<agbpack::code_table_entry>
{
    static std::string convert(const agbpack::code_table_entry& e)
    {
        return std::format("s={}, c={}, l={}", e.s(), e.c(), e.l());
    }
};

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
        CHECK(code_table[0] == code_table_entry(0, 0b11111111111111111111111111111010, 32));
        CHECK(code_table[1] == code_table_entry(1, 0b11111111111111111111111111111011, 32));
        CHECK(code_table[2] == code_table_entry(2, 0b1111111111111111111111111111100, 31));
        CHECK(code_table[3] == code_table_entry(3, 0b111111111111111111111111111111, 30));
        CHECK(code_table[4] == code_table_entry(4, 0b11111111111111111111111111110, 29));
        CHECK(code_table[5] == code_table_entry(5, 0b1111111111111111111111111110, 28));
        /*
27  ==>  134217726
26  ==>  67108862
25  ==>  33554430
24  ==>  16777214
23  ==>  8388606
22  ==>  4194302
21  ==>  2097150
20  ==>  1048574
19  ==>  524286
18  ==>  262142
17  ==>  131070
16  ==>  65534
15  ==>  32766
14  ==>  16382
13  ==>  8190
12  ==>  4094
11  ==>  2046
10  ==>  1022
9  ==>  510
8  ==>  254
7  ==>  126
6  ==>  62
5  ==>  30
4  ==>  14
3  ==>  6
2  ==>  2
1  ==>  0        */
    }

    SECTION("Maximum code length exceeded")
    {
        // TODO: create_code_table: maximum code length exceeded
    }
}

}

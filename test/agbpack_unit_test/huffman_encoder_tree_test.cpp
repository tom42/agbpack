// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <format>
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

constexpr auto symbol_size = 8;

using agbpack::code_table_entry;
using agbpack::encode_exception;
using agbpack::frequency_table;
using agbpack::huffman_encoder_tree;
using agbpack::max_code_length;
using agbpack::symbol;

namespace
{

huffman_encoder_tree create_tree_from_lucas_sequence(std::size_t sequence_length)
{
    auto sequence = lucas_sequence(sequence_length);
    frequency_table frequencies(symbol_size);

    for (symbol i = 0; i < sequence.size(); ++i)
    {
        frequencies.set_frequency(i, sequence[i]);
    }

    return huffman_encoder_tree(symbol_size, frequencies);
}

}

TEST_CASE("huffman_encoder_tree_test")
{
    SECTION("Create code table with maximum code length")
    {
        huffman_encoder_tree tree = create_tree_from_lucas_sequence(max_code_length + 1);

        auto code_table = tree.create_code_table();

        CHECK(code_table[ 0] == code_table_entry( 0, 0b11111111111111111111111111111010, 32));
        CHECK(code_table[ 1] == code_table_entry( 1, 0b11111111111111111111111111111011, 32));
        CHECK(code_table[ 2] == code_table_entry( 2, 0b1111111111111111111111111111100, 31));
        CHECK(code_table[ 3] == code_table_entry( 3, 0b111111111111111111111111111111, 30));
        CHECK(code_table[ 4] == code_table_entry( 4, 0b11111111111111111111111111110, 29));
        CHECK(code_table[ 5] == code_table_entry( 5, 0b1111111111111111111111111110, 28));
        CHECK(code_table[ 6] == code_table_entry( 6, 0b111111111111111111111111110, 27));
        CHECK(code_table[ 7] == code_table_entry( 7, 0b11111111111111111111111110, 26));
        CHECK(code_table[ 8] == code_table_entry( 8, 0b1111111111111111111111110, 25));
        CHECK(code_table[ 9] == code_table_entry( 9, 0b111111111111111111111110, 24));
        CHECK(code_table[10] == code_table_entry(10, 0b11111111111111111111110, 23));
        CHECK(code_table[11] == code_table_entry(11, 0b1111111111111111111110, 22));
        CHECK(code_table[12] == code_table_entry(12, 0b111111111111111111110, 21));
        CHECK(code_table[13] == code_table_entry(13, 0b11111111111111111110, 20));
        CHECK(code_table[14] == code_table_entry(14, 0b1111111111111111110, 19));
        CHECK(code_table[15] == code_table_entry(15, 0b111111111111111110, 18));
        CHECK(code_table[16] == code_table_entry(16, 0b11111111111111110, 17));
        CHECK(code_table[17] == code_table_entry(17, 0b1111111111111110, 16));
        CHECK(code_table[18] == code_table_entry(18, 0b111111111111110, 15));
        CHECK(code_table[19] == code_table_entry(19, 0b11111111111110, 14));
        CHECK(code_table[20] == code_table_entry(20, 0b1111111111110, 13));
        CHECK(code_table[21] == code_table_entry(21, 0b111111111110, 12));
        CHECK(code_table[22] == code_table_entry(22, 0b11111111110, 11));
        CHECK(code_table[23] == code_table_entry(23, 0b1111111110, 10));
        CHECK(code_table[24] == code_table_entry(24, 0b111111110, 9));
        CHECK(code_table[25] == code_table_entry(25, 0b11111110, 8));
        CHECK(code_table[26] == code_table_entry(26, 0b1111110, 7));
        CHECK(code_table[27] == code_table_entry(27, 0b111110, 6));
        CHECK(code_table[28] == code_table_entry(28, 0b11110, 5));
        CHECK(code_table[29] == code_table_entry(29, 0b1110, 4));
        CHECK(code_table[30] == code_table_entry(30, 0b110, 3));
        CHECK(code_table[31] == code_table_entry(31, 0b10, 2));
        CHECK(code_table[32] == code_table_entry(32, 0b0, 1));
    }

    SECTION("Maximum code length exceeded")
    {
        huffman_encoder_tree tree = create_tree_from_lucas_sequence(max_code_length + 2);

        CHECK_THROWS_MATCHES(
            tree.create_code_table(),
            encode_exception,
            Catch::Matchers::Message("maximum code length exceeded"));
    }
}

}

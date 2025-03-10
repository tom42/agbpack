// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <vector>

import agbpack;
import agbpack_unit_testkit;

namespace agbpack_unit_test
{

using agbpack::byte_reader;
using agbpack::code_table;
using agbpack::frequency_table;
using agbpack::get_nsymbols;
using agbpack::huffman_decoder_tree;
using agbpack::huffman_encoder_tree;
using agbpack::huffman_tree_serializer;
using agbpack::max_code_length;
using agbpack::symbol;
using agbpack_unit_testkit::create_tree_from_lucas_sequence;
using std::out_of_range;
using std::size_t;
using std::vector;

namespace
{

// As far as tree serialization goes there is little difference between 4 and 8 bit wide symbols,
// except that 4 bit wide symbols are not prone to overflows in the offset field of internal nodes.
// It is therefore enough if we test with 8 bit wide symbols. 4 bit wide symbols are not interesting.
constexpr unsigned int symbol_size = 8;

auto serialize_tree(const huffman_encoder_tree& tree)
{
    huffman_tree_serializer serializer;
    return serializer.serialize(tree);
}

auto deserialize_tree(const vector<unsigned char>& serialized_tree)
{
    byte_reader reader(begin(serialized_tree), end(serialized_tree));
    return huffman_decoder_tree(symbol_size, reader);
}

auto create_and_serialize_tree(const frequency_table& frequencies)
{
    huffman_encoder_tree tree(symbol_size, frequencies);
    return serialize_tree(tree);
}

size_t expected_serialized_tree_size(const huffman_encoder_tree& encoder_tree)
{
    size_t size = encoder_tree.root()->num_leaves() * 2 - 1;

    while (size % 4 != 0)
    {
        ++size;
    }

    return size;
}

void verify_tree_serialization(const huffman_encoder_tree& encoder_tree)
{
    const code_table original_code_table = encoder_tree.create_code_table();

    const auto serialized_tree = serialize_tree(encoder_tree);
    const huffman_decoder_tree deserialized_tree = deserialize_tree(serialized_tree);
    const code_table deserialized_code_table = deserialized_tree.create_code_table();

    REQUIRE(serialized_tree.size() == expected_serialized_tree_size(encoder_tree));

    for (symbol i = 0; i < get_nsymbols(symbol_size); ++i)
    {
        REQUIRE(deserialized_code_table[i].s() == original_code_table[i].s());
        REQUIRE(deserialized_code_table[i].c() == original_code_table[i].c());
        REQUIRE(deserialized_code_table[i].l() == original_code_table[i].l());
    }
}

}

TEST_CASE("huffman_tree_serializer_test")
{
    frequency_table frequencies(symbol_size);

    SECTION("No symbols")
    {
        const vector<unsigned char> expected_serialized_tree = {0x01, 0xc0, 0x00, 0x00};

        auto serialized_tree = create_and_serialize_tree(frequencies);

        CHECK(serialized_tree == expected_serialized_tree);
    }

    SECTION("One symbol")
    {
        frequencies.set_frequency('a', 1);
        const vector<unsigned char> expected_serialized_tree = {0x01, 0xc0, 0x00, 'a'};

        auto serialized_tree = create_and_serialize_tree(frequencies);

        CHECK(serialized_tree == expected_serialized_tree);
    }

    SECTION("Two symbols")
    {
        frequencies.set_frequency('a', 1);
        frequencies.set_frequency('b', 1);
        const vector<unsigned char> expected_serialized_tree = { 0x01, 0xc0, 'a', 'b'};

        auto serialized_tree = create_and_serialize_tree(frequencies);

        CHECK(serialized_tree == expected_serialized_tree);
    }

    SECTION("Three symbols")
    {
        frequencies.set_frequency('a', 1);
        frequencies.set_frequency('b', 1);
        frequencies.set_frequency('c', 1);
        const vector<unsigned char> expected_serialized_tree = { 0x03, 0x80, 'c', 0xc0, 'a', 'b', 0x00, 0x00};

        auto serialized_tree = create_and_serialize_tree(frequencies);

        CHECK(serialized_tree == expected_serialized_tree);
    }

    SECTION("Nonzero offsets")
    {
        frequencies.set_frequency('a', 1);
        frequencies.set_frequency('b', 2);
        frequencies.set_frequency('c', 2);
        frequencies.set_frequency('d', 2);
        frequencies.set_frequency('e', 2);
        frequencies.set_frequency('f', 2);
        frequencies.set_frequency('g', 3);
        frequencies.set_frequency('h', 4);
        const vector<unsigned char> expected_serialized_tree =
        {
            0x07, 0x00, 0x00, 0x81, 0xc1, 0xc2, 'h', 0x42, 'c', 'd', 'e', 'f', 0xc0, 'g', 'a', 'b'
        };

        auto serialized_tree = create_and_serialize_tree(frequencies);

        CHECK(serialized_tree == expected_serialized_tree);
    }

    SECTION("256 symbols with same frequency")
    {
        for (symbol i = 0; i < 256; ++i)
        {
            frequencies.set_frequency(i, 1);
        }
        huffman_encoder_tree tree(symbol_size, frequencies);

        verify_tree_serialization(tree);
    }

    SECTION("Maximum depth tree")
    {
        huffman_encoder_tree tree = create_tree_from_lucas_sequence(max_code_length + 1);

        verify_tree_serialization(tree);
    }
}

}

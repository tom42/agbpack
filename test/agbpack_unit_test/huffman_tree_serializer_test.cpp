// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <vector>

import agbpack;

namespace agbpack_unit_test
{

using std::vector;
using agbpack::byte_reader;
using agbpack::code_table;
using agbpack::frequency_table;
using agbpack::huffman_decoder_tree;
using agbpack::huffman_encoder_tree;
using agbpack::huffman_tree_serializer;

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

void verify_tree_serialization(const frequency_table& frequencies)
{
    const huffman_encoder_tree encoder_tree(symbol_size, frequencies);
    const code_table original_code_table = encoder_tree.create_code_table();

    const auto serialized_tree = serialize_tree(encoder_tree); // TODO: check size of serialized tree? (We can calculate it from the symbol size, no? No. We can calculate it from the total number of nodes)
    const huffman_decoder_tree deserialized_tree = deserialize_tree(serialized_tree);
    const code_table deserialized_code_table = deserialized_tree.create_code_table();

    for (unsigned int i = 0; i < 256; ++i)
    {
        // TODO: this creates a ridiculously high assertion count. do we care?
        REQUIRE(deserialized_code_table[i].c() == original_code_table[i].c());
        REQUIRE(deserialized_code_table[i].l() == original_code_table[i].l());
    }
}

}

// TODO: tests
//       * Tree with 256 symbols with same frequency
//       * Maximum depth tree
//       * Maximum handleable code length exceeded
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
        for (unsigned int i = 0; i < 256; ++i)
        {
            frequencies.set_frequency(i, 1);
        }

        verify_tree_serialization(frequencies);
    }
}

}

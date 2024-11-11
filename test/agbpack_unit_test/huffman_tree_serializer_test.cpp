// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <vector>

import agbpack;

namespace agbpack_unit_test
{

using std::vector;
using agbpack::frequency_table;
using agbpack::huffman_encoder_tree;
using agbpack::huffman_tree_serializer;

namespace
{

auto create_and_serialize_tree(const frequency_table& frequencies)
{
    huffman_encoder_tree tree(8, frequencies); // TODO: obtain symbol size from frequency table?
    huffman_tree_serializer serializer;
    return serializer.serialize(tree);
}

}

// TODO: tests
//       * Tree with 256 symbols with same frequency
//       * Maximum depth tree
//       * Maximum handleable code length exceeded
TEST_CASE("huffman_tree_serializer_test")
{
    frequency_table frequencies(8);

    SECTION("No symbols")
    {
        const vector<unsigned char> expected_serialized_tree = {0x01, 0xc0, 0x00, 0x00};

        auto serialized_tree = create_and_serialize_tree(frequencies);

        CHECK(serialized_tree == expected_serialized_tree);
    }

    SECTION("One symbol")
    {
        frequencies.set_frequency('A', 1);
        const vector<unsigned char> expected_serialized_tree = {0x01, 0xc0, 0x00, 'A'};

        auto serialized_tree = create_and_serialize_tree(frequencies);

        CHECK(serialized_tree == expected_serialized_tree);
    }

    SECTION("Two symbols")
    {
        frequencies.set_frequency('A', 1);
        frequencies.set_frequency('B', 1);
        const vector<unsigned char> expected_serialized_tree = { 0x01, 0xc0, 'A', 'B'};

        auto serialized_tree = create_and_serialize_tree(frequencies);

        CHECK(serialized_tree == expected_serialized_tree);
    }

    SECTION("Three symbols")
    {
        frequencies.set_frequency('A', 1);
        frequencies.set_frequency('B', 1);
        frequencies.set_frequency('C', 1);
        const vector<unsigned char> expected_serialized_tree = { 0x03, 0x80, 'C', 0xc0, 'A', 'B', 0x00, 0x00};

        auto serialized_tree = create_and_serialize_tree(frequencies);

        CHECK(serialized_tree == expected_serialized_tree);
    }

    SECTION("Nonzero offsets")
    {
        frequencies.set_frequency('A', 1);
        frequencies.set_frequency('B', 2);
        frequencies.set_frequency('C', 2);
        frequencies.set_frequency('D', 2);
        frequencies.set_frequency('E', 2);
        frequencies.set_frequency('F', 2);
        frequencies.set_frequency('G', 3);
        frequencies.set_frequency('H', 4);
        const vector<unsigned char> expected_serialized_tree =
        {
            0x07, 0x00, 0x00, 0x81, 0xc1, 0xc2, 'H', 0x82, 'F', 'B', 'E', 'D', 'G', 0xc0, 'A', 'C'
        };

        auto serialized_tree = create_and_serialize_tree(frequencies);

        CHECK(serialized_tree == expected_serialized_tree);
    }
}

}

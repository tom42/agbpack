// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cassert>
#include <iterator>
#include <vector>

export module agbpack:huffman;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

export class huffman_decoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type(input);

        byte_reader<InputIterator> reader(input, eof);
        auto header = header::parse_for_type(compression_type::huffman, reader.read32());
        if (!header)
        {
            throw bad_encoded_data();
        }

        // TODO: actually calculate and store tree size
        // TODO: also document a bit how to interpret this?
        //       Basically it points at the bitstream?
        // TODO: probably we want to read the entire tree including the tree size into a vector...
        read_huffman_tree(reader);

        // TODO: read huffman tree (what sizes do we support? => depends mostly on what the BIOS can do)

        // TODO: decode data (what sizes do we support?)
        //       * Note: Quote: "Compressed Bitstream (stored in units of 32bits)"
        //       * This means that
        //         * We need to read 32 bit units into our bit buffer
        //         * Data is automatically aligned, so there should never be any padding bytes
        //         * ??? There might be padding between the huffman tree and the bit stream ???
        *output++ = 'a';
        *output++ = 'b';

        // TODO: parse padding bytes here
    }
private:

    template <std::input_iterator InputIterator>
    static std::vector<agbpack_u8> read_huffman_tree(byte_reader<InputIterator>& reader)
    {
        // TODO: actually read huffman tree
        // TODO: document tree size and the tree a bit
        // TODO: testcase: EOF while reading rest of tree (tree size is already covered)
        // TODO: it may be advantageous to store the tree size byte in the tree. If so, do so.
        //       => Take care to store the original byte, not the true tree size (it won't fit into 8 bits)
        // TODO: tests: minimum/maximum tree size?
        std::vector<agbpack_u8> tree;
        auto tree_size = 2 * (reader.read8() + 1);
        assert((2 <= tree_size) && (tree_size <= 512) && "huffman_decoder is broken");

        // TODO: read in remaining bits of tree. Note that tree size counts toward the tree, so read one byte less
        // TODO: test: EOF when reading tree
        // TODO: do we need to align anything here? I think so, no? After all, the bit stream needs to be 4 byte aligned no?
        for (int i = 0; i < tree_size - 1; ++i)
        {
            tree.push_back(reader.read8());
        }

        return tree;
    }
};

}

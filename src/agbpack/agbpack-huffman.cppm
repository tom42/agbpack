// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cassert>
#include <iterator>
#include <utility>
#include <vector>

export module agbpack:huffman;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

inline int get_symbol_size(header h)
{
    return std::to_underlying(h.options_as<huffman_options>());
}

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

        // TODO: also document a bit how to interpret this?
        //       Basically it points at the bitstream?
        read_huffman_tree(reader);

        // TODO: read huffman tree (what sizes do we support? => depends mostly on what the BIOS can do)

        // TODO: decode data (what sizes do we support?)
        //       * Note: Quote: "Compressed Bitstream (stored in units of 32bits)"
        //       * This means that
        //         * We need to read 32 bit units into our bit buffer
        //         * Data is automatically aligned, so there should never be any padding bytes
        //         * ??? There might be padding between the huffman tree and the bit stream ???
        const int symbol_size = get_symbol_size(*header);
        byte_writer<OutputIterator> writer(header->uncompressed_size(), output);

        while (!writer.done())
        {
            agbpack_u8 decoded_byte = 0;
            for (int nbits = 0; nbits < 8; nbits += symbol_size)
            {
                decoded_byte |= decode_symbol(symbol_size) << nbits;
            }

            // TODO: when writing an output byte, test buffer overrun on output buffer
            writer.write8(decoded_byte);
        }

        // TODO: parse padding bytes here
    }
private:
    // TODO: remove symbol size argument
    agbpack_u8 decode_symbol(int symbol_size)
    {
        // TODO: actually decode stuff
        if (symbol_size == 8)
        {
            std::vector<agbpack_u8> buf{ 'a', 'b' };
            return static_cast<agbpack_u8>(buf[m_cnt++]);
        }
        else
        {
            std::vector<agbpack_u8> arr{ 'a' & 15, 'a' >> 4, 'b' & 15, 'b' >> 4 };
            return arr[m_cnt++];
        }
    }

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

    std::size_t m_cnt = 0; // TODO: remove
};

}

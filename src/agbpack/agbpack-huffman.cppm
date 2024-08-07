// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cassert>
#include <cstdint>
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

template <std::input_iterator InputIterator>
class bitstream_reader final
{
public:
    bitstream_reader(const bitstream_reader&) = delete;
    bitstream_reader& operator=(const bitstream_reader&) = delete;

    explicit bitstream_reader(byte_reader<InputIterator>& byte_reader)
        : m_byte_reader(byte_reader)
    {}

    bool get_bit()
    {
        m_bitmask >>= 1;
        if (!m_bitmask)
        {
            m_bitmask = 0x80000000;
            // TODO: test case: eof while refilling the bit buffer
            m_bitbuffer = m_byte_reader.read32();
        }

        return m_bitbuffer & m_bitmask;
    }

private:
    std::uint32_t m_bitbuffer = 0;
    std::uint32_t m_bitmask = 0;
    byte_reader<InputIterator>& m_byte_reader;
};

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

        auto huffman_tree = read_huffman_tree(reader);

        // TODO: read huffman tree (what sizes do we support? => depends mostly on what the BIOS can do)

        // TODO: decode data (what sizes do we support?)
        //       * Note: Quote: "Compressed Bitstream (stored in units of 32bits)"
        //       * This means that
        //         * We need to read 32 bit units into our bit buffer
        //         * Data is automatically aligned, so there should never be any padding bytes
        //         * ??? There might be padding between the huffman tree and the bit stream ???
        const int symbol_size = get_symbol_size(*header);
        bitstream_reader<InputIterator> bit_reader(reader);
        byte_writer<OutputIterator> writer(header->uncompressed_size(), output);

        while (!writer.done())
        {
            agbpack_u8 decoded_byte = 0;
            for (int nbits = 0; nbits < 8; nbits += symbol_size)
            {
                decoded_byte |= decode_symbol(bit_reader, huffman_tree) << nbits;
            }

            // TODO: when writing an output byte, test buffer overrun on output buffer
            writer.write8(decoded_byte);
        }

        // TODO: parse padding bytes here
    }
private:
    // TODO: consider putting this elsewhere? => huffman_tree class
    template <std::input_iterator InputIterator>
    agbpack_u8 decode_symbol(bitstream_reader<InputIterator>& bit_reader, const std::vector<agbpack_u8>& huffman_tree)
    {
        constexpr auto mask_left = 0x80;
        constexpr auto mask_right = 0x40;
        constexpr auto mask_next_node_offset = 63;

        bool character_found = false;
        std::size_t current_node_index = 0;
        auto current_node_value = huffman_tree[1]; // TODO: test: out of bounds access of huffman tree? Note: smallest good index is 1

        while (!character_found)
        {
            current_node_index += 2 * ((current_node_value & mask_next_node_offset) + 1);

            if (!bit_reader.get_bit())
            {
                character_found = current_node_value & mask_left;
                current_node_value = huffman_tree[current_node_index]; // TODO: test: out of bounds access of huffman tree? Note: smallest good index is 1
            }
            else
            {
                character_found = current_node_value & mask_right;
                current_node_value = huffman_tree[current_node_index + 1]; // TODO: test: out of bounds access of huffman tree? Note: smallest good index is 1
            }
        }

        // TODO: fail if there is garbage in the upper unused bits of the node. They should be 0.
        // TODO: test what happens if there is garbage

        return current_node_value;
    }

    template <std::input_iterator InputIterator>
    static std::vector<agbpack_u8> read_huffman_tree(byte_reader<InputIterator>& reader)
    {
        // TODO: document tree size and the tree a bit
        //       => Maybe also document the wording from gbatek
        // TODO: testcase: EOF while reading rest of tree (tree size is already covered)
        // TODO: tests: minimum/maximum tree size?
        std::vector<agbpack_u8> tree;

        std::size_t tree_size = 2 * (reader.read8() + 1);
        assert((2 <= tree_size) && (tree_size <= 512) && "huffman_decoder is broken");

        // The address calculations as documented in GBATEK and implemented in decode_symbol
        // work relative to the address of the tree size byte. It is therefore simplest if we
        // put a byte in front of our huffman tree in memory. The value of that byte does not matter.
        tree.push_back(0);

        // TODO: test: EOF when reading tree
        // TODO: do we need to align anything here? I think so, no? After all, the bit stream needs to be 4 byte aligned no?

        // Read huffman tree. Note that the tree size byte counts towards the tree size.
        // Obviously we have already read the tree size byte, so we need to read one byte
        // less than the value in tree_size.
        reader.read8(tree_size - 1, back_inserter(tree));

        return tree;
    }
};

}

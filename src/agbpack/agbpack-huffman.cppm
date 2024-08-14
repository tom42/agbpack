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
            // The encoded bitstream is stored in units of 32 bits, with padding bits at the
            // end of the stream. Within a 32 bit unit, the MSB is to be processed first.
            // So we read 4 bytes from the byte stream whenever our bit buffer is empty.
            m_bitmask = 0x80000000;
            m_bitbuffer = m_byte_reader.read32();
        }

        return m_bitbuffer & m_bitmask;
    }

private:
    std::uint32_t m_bitbuffer = 0;
    std::uint32_t m_bitmask = 0;
    byte_reader<InputIterator>& m_byte_reader;
};

template <std::input_iterator InputIterator>
class huffman_tree final
{
public:
    explicit huffman_tree(byte_reader<InputIterator>& reader)
    {
        read_tree(reader);
    }

    agbpack_u8 decode_symbol(bitstream_reader<InputIterator>& bit_reader)
    {
        constexpr auto mask_left = 0x80;
        constexpr auto mask_right = 0x40;
        constexpr auto mask_next_node_offset = 63;

        bool character_found = false;
        std::size_t current_node_index = 0;
        auto current_node_value = m_tree[1]; // TODO: test: out of bounds access of huffman tree? Note: smallest good index is 1

        while (!character_found)
        {
            current_node_index += 2 * ((current_node_value & mask_next_node_offset) + 1);

            if (!bit_reader.get_bit())
            {
                character_found = current_node_value & mask_left;
                current_node_value = m_tree[current_node_index]; // TODO: test: out of bounds access of huffman tree? Note: smallest good index is 1
            }
            else
            {
                character_found = current_node_value & mask_right;
                current_node_value = m_tree[current_node_index + 1]; // TODO: test: out of bounds access of huffman tree? Note: smallest good index is 1
            }
        }

        // TODO: fail if there is garbage in the upper unused bits of the node. They should be 0.
        // TODO: test what happens if there is garbage

        return current_node_value;
    }


private:
    void read_tree(byte_reader<InputIterator>& reader)
    {
        // TODO: tests: minimum tree size?

        // Read tree size byte and calculate tree size from that.
        //
        // Quote from GBATEK: "Size of Tree Table/2-1 (ie. Offset to Compressed Bitstream)"
        //
        // Now "size of table" and "offset to bitstream" are two rather different things.
        // Probably the latter is the correct interpretation, meaning that there might be
        // padding bytes between the tree data and the bitstream. That in turn would make
        // sense, since the format seems to be designed such that the bitstream can be
        // processed in units of 32 bits by an ARM CPU.
        std::size_t tree_size = 2 * (reader.read8() + 1);
        assert((2 <= tree_size) && (tree_size <= 512) && "huffman_decoder is broken");

        // The address calculations as documented in GBATEK and implemented in decode_symbol
        // work relative to the address of the tree size byte. It is therefore simplest if we
        // put a byte in front of our huffman tree in memory. The value of that byte does not matter.
        m_tree.push_back(0);

        // TODO: do we need to align anything here? I think so, no? After all, the bit stream needs to be 4 byte aligned no?
        //       => No but we might want to check whether we ARE aligned. But actually the decoder can do that too

        // Read huffman tree. Note that the tree size byte counts towards the tree size.
        // Obviously we have already read the tree size byte, so we need to read one byte
        // less than the value in tree_size.
        reader.read8(tree_size - 1, back_inserter(m_tree));
    }

    std::vector<agbpack_u8> m_tree;
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

        huffman_tree<InputIterator> tree(reader);

        // TODO: do we check here whether the bitstream is aligned at a 4 byte boundary?

        const int symbol_size = get_symbol_size(*header);
        bitstream_reader<InputIterator> bit_reader(reader);
        byte_writer<OutputIterator> writer(header->uncompressed_size(), output);

        while (!writer.done())
        {
            agbpack_u8 decoded_byte = 0;
            for (int nbits = 0; nbits < 8; nbits += symbol_size)
            {
                decoded_byte |= tree.decode_symbol(bit_reader) << nbits;
            }

            // TODO: when writing an output byte, test buffer overrun on output buffer
            writer.write8(decoded_byte);
        }

        // TODO: parse padding bytes here
    }
};

}

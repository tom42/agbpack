// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cassert>
#include <cstdint>
#include <iterator>
#include <memory>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>

export module agbpack:huffman;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

inline unsigned int get_symbol_size(huffman_options options)
{
    return std::to_underlying(options);
}

inline unsigned int get_nsymbols(unsigned int symbol_size)
{
    return 1 << symbol_size;
}

inline unsigned int get_symbol_mask(unsigned int symbol_size)
{
    return get_nsymbols(symbol_size) - 1;
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
            m_bitbuffer = read32(m_byte_reader);
        }

        return m_bitbuffer & m_bitmask;
    }

private:
    std::uint32_t m_bitbuffer = 0;
    std::uint32_t m_bitmask = 0;
    byte_reader<InputIterator>& m_byte_reader;
};

template <std::input_iterator InputIterator>
class huffman_decoder_tree final
{
public:
    explicit huffman_decoder_tree(unsigned int symbol_size, byte_reader<InputIterator>& reader)
        : m_symbol_max_value((1 << symbol_size) - 1u)
    {
        read_tree(reader);
    }

    agbpack_u8 decode_symbol(bitstream_reader<InputIterator>& bit_reader) const
    {
        constexpr auto mask_left = 0x80;
        constexpr auto mask_right = 0x40;
        constexpr auto mask_next_node_offset = 63;

        bool character_found = false;
        std::size_t current_node_index = 0;
        auto current_node_value = read_tree_node(root_node_index);

        while (!character_found)
        {
            current_node_index += 2u * ((current_node_value & mask_next_node_offset) + 1);

            if (!bit_reader.get_bit())
            {
                character_found = current_node_value & mask_left;
                current_node_value = read_tree_node(current_node_index);
            }
            else
            {
                character_found = current_node_value & mask_right;
                current_node_value = read_tree_node(current_node_index + 1);
            }
        }

        if (current_node_value > m_symbol_max_value)
        {
            throw decode_exception();
        }

        return current_node_value;
    }


private:
    void read_tree(byte_reader<InputIterator>& reader)
    {
        // Read tree size byte and calculate tree size from that.
        //
        // Quote from GBATEK: "Size of Tree Table/2-1 (ie. Offset to Compressed Bitstream)"
        //
        // Now "size of table" and "offset to bitstream" are two rather different things.
        // Probably the latter is the correct interpretation, meaning that there might be
        // padding bytes between the tree data and the bitstream. That in turn would make
        // sense, since the format seems to be designed such that the bitstream can be
        // processed in units of 32 bits by an ARM CPU.
        std::size_t tree_size = 2u * (read8(reader) + 1);
        assert((2 <= tree_size) && (tree_size <= 512) && "huffman_decoder is broken");

        // The address calculations as documented in GBATEK and implemented in decode_symbol
        // work relative to the address of the tree size byte. It is therefore simplest if we
        // put a byte in front of our huffman tree in memory. The value of that byte does not matter.
        m_tree.push_back(0);

        // Read huffman tree. Note that the tree size byte counts towards the tree size.
        // Obviously we have already read the tree size byte, so we need to read one byte
        // less than the value in tree_size.
        read8(reader, tree_size - 1, back_inserter(m_tree));
    }

    auto read_tree_node(std::size_t node_index) const
    {
        if ((node_index < root_node_index) || (node_index >= m_tree.size()))
        {
            throw decode_exception();
        }

        return m_tree[node_index];
    }

    static constexpr std::size_t root_node_index = 1;
    unsigned int m_symbol_max_value;
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
        auto header = header::parse_for_type(compression_type::huffman, read32(reader));
        if (!header)
        {
            throw decode_exception();
        }

        const unsigned int symbol_size = get_symbol_size(header->template options_as<huffman_options>());
        huffman_decoder_tree<InputIterator> tree(symbol_size, reader);

        throw_if_bitstream_is_misaligned(reader);

        bitstream_reader<InputIterator> bit_reader(reader);
        byte_writer<OutputIterator> writer(header->uncompressed_size(), output);

        while (!writer.done())
        {
            agbpack_u8 decoded_byte = 0;
            for (unsigned int nbits = 0; nbits < 8; nbits += symbol_size)
            {
                decoded_byte |= tree.decode_symbol(bit_reader) << nbits;
            }

            write8(writer, decoded_byte);
        }

        // We already checked whether the bitstream is aligned, and we read it 32 bit wise.
        // So if at this point we're not 32 bit aligned, then the decoder is broken.
        assert(((reader.nbytes_read() % 4) == 0) && "huffman_decoder is broken");
    }

private:
    template <std::input_iterator InputIterator>
    static void throw_if_bitstream_is_misaligned(const byte_reader<InputIterator>& reader)
    {
        if ((reader.nbytes_read() % 4) != 0)
        {
            throw decode_exception();
        }
    }
};

using symbol = agbpack_u8;
using symbol_frequency = uint32_t;

class frequency_table final
{
public:
    explicit frequency_table(unsigned int symbol_size)
        : m_symbol_size(symbol_size)
        , m_frequencies(get_nsymbols(symbol_size))
    {}

    template <std::input_iterator InputIterator>
    std::vector<agbpack_u8> update(InputIterator input, InputIterator eof)
    {
        std::vector<agbpack_u8> data;
        byte_reader<InputIterator> reader(input, eof);
        auto symbol_mask = get_symbol_mask(m_symbol_size);

        while (!reader.eof())
        {
            auto byte = reader.read8();
            data.push_back(byte);

            for (unsigned int nbits = 0; nbits < 8; nbits += m_symbol_size)
            {
                auto symbol = byte & symbol_mask;
                ++m_frequencies[symbol];
                byte >>= m_symbol_size;
            }
        }

        return data;
    }

private:
    unsigned int m_symbol_size;
    std::vector<symbol_frequency> m_frequencies;
};

// TODO: figure out what this needs:
//       * frequency?
//       * symbol?
//       * Left child
//       * Right child
// TODO: forbid copying of this class?
// TODO: how do we easily tell leaf nodes from non-leaf nodes? Invalid symbol value or dedicated flag?
// TODO: document why shared_ptr and not unique_ptr?
class tree_node final
{
public:
    tree_node(symbol symbol, symbol_frequency frequency)
        : m_symbol(symbol)
        , m_frequency(frequency)
    {
        // TODO: remove suppressions (2x)
        (void)m_symbol;
        (void)m_frequency;
    }

private:
    symbol m_symbol;
    symbol_frequency m_frequency;
    std::shared_ptr<tree_node> m_left;
    std::shared_ptr<tree_node> m_right;
};

class huffman_encoder_tree final
{
public:
    explicit huffman_encoder_tree(unsigned int /*symbol_size*/, const frequency_table& /*ftable*/)
    {
        // TODO: create tree: decision: what algorithm to use?
        //       * Classic: create a bunch of nodes on some sort of heap/queue. Note: C++ has a priority queue
        //       * Do as in The Data Compression Book (not sure this is really better)
        std::priority_queue<std::shared_ptr<tree_node>> meh; // TODO: name. Just experimenting: can we create one?
        meh.push(std::make_shared<tree_node>('a', 23));
        meh.push(std::make_shared<tree_node>('b', 11));
        meh.push(std::make_shared<tree_node>('c', 13));

        while (meh.size())
        {
        }
    }
private:
};

export class huffman_encoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void encode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type(input);

        const unsigned int symbol_size = get_symbol_size(m_options);

        // Create frequency table.
        // We need to re-read the input during encoding, so we also create a buffer with the input.
        frequency_table ftable(symbol_size);
        ftable.update(input, eof);

        huffman_encoder_tree tree(symbol_size, ftable);

        // TODO: as usual, need to encode stuff to temporary buffer
        // TODO: actually encode stuff
        //       * create tree/codes from frequency table
        //       * serialize tree to output
        //       * encode data to output

        // TODO: unhardcode uncompressed size
        // TODO: check uncompressed size and throw appropriate exception if too big
        auto header = header::create(m_options, 0);

        // Copy header and encoded data to output
        unbounded_byte_writer<OutputIterator> writer(output);
        write32(writer, header.to_uint32_t());

        // TODO: unhardcode tree data
        writer.write8(1);
        writer.write8(0xc0);
        writer.write8(0);
        writer.write8(0);

        // TODO: write encoded data
    }

    void options(huffman_options options)
    {
        if (!is_valid(options))
        {
            throw std::invalid_argument("invalid huffman compression options");
        }

        m_options = options;
    }

private:
    huffman_options m_options = huffman_options::h8;
};

}

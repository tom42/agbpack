// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <algorithm>
#include <cassert>
#include <cctype>
#include <concepts>
#include <cstdint>
#include <format>
#include <iostream>
#include <iterator>
#include <memory>
#include <queue>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

export module agbpack:huffman;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

using symbol = unsigned int; // TODO: this should not be agbpack_u8. Document this / write test that would figure out?
using symbol_frequency = uint32_t;
using code = unsigned int;
using code_length = unsigned int;

inline constexpr auto max_next_node_offset = 63;
inline constexpr auto mask_next_node_offset = 63;
inline constexpr auto max_serialized_tree_size = 512;
inline constexpr auto mask0 = 0x80;
inline constexpr auto mask1 = 0x40;

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

template <std::integral T>
bool in_range(T x, T min, T max)
{
    return (min <= x) && (x <= max);
}

template <typename TContainer>
void assert_symbol(symbol s, [[maybe_unused]] const TContainer& container)
{
    // TODO: verify assertion is correct
    static_assert(std::is_unsigned_v<decltype(s)>, "Must also assert that s is >= 0 if s is not of unsigned type");
    assert(s < container.size());
}

class code_table_entry final
{
public:
    code_table_entry() : code_table_entry(0, 0, 0) {}

    code_table_entry(symbol s, code c, code_length l)
        : m_s(s)
        , m_c(c)
        , m_l(l)
    {}

    symbol s() const { return m_s; }

    code c() const { return m_c; }

    code_length l() const { return m_l; }

private:
    symbol m_s;
    code m_c;
    code_length m_l;
};

class code_table final
{
public:
    code_table(unsigned int symbol_size)
        : m_symbol_size(symbol_size)
        , m_table(get_nsymbols(symbol_size))
    {}

    unsigned int symbol_size() const
    {
        return m_symbol_size;
    }

    void set(symbol s, code c, code_length l)
    {
        assert_symbol(s, m_table);
        m_table[s] = code_table_entry(s, c, l);
    }

    const code_table_entry& operator[](symbol s) const
    {
        assert_symbol(s, m_table);
        return m_table[s];
    }

    void dump() const
    {
        auto sorted_table = m_table;
        std::sort(sorted_table.begin(), sorted_table.end(), compare_code_table_entry());

        for (const auto& entry : sorted_table)
        {
            if (entry.l() > 0)
            {
                // TODO: replace static_cast<int> with something else: use make_signed or something from agbpack-lzss.cppm
                char symbol_as_char = std::isprint(static_cast<int>(entry.s())) ? static_cast<char>(entry.s()) : '?';
                std::cout << std::format("{:3} {}: {:0{}b}\n", entry.s(), symbol_as_char, entry.c(), entry.l());
            }
        }
    }

private:
    struct compare_code_table_entry final
    {
        bool operator()(const code_table_entry& a, const code_table_entry& b)
        {
            // Sort by code length
            if (a.l() != b.l())
            {
                return a.l() < b .l();
            }

            // Then by code
            return a.c() < b.c();
        }
    };

    unsigned int m_symbol_size;
    std::vector<code_table_entry> m_table;
};

template <std::input_iterator InputIterator>
class bitstream_reader final
{
public:
    bitstream_reader(const bitstream_reader&) = delete;
    bitstream_reader& operator=(const bitstream_reader&) = delete;

    explicit bitstream_reader(byte_reader<InputIterator>& byte_reader)
        : m_byte_reader(byte_reader)
    {}

    bool read_bit()
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

// TODO: overhaul this:
//       * It's rather ugly
//       * write_bit should possibly be private
//       * It has the constant 0x80000000 as a magic in several places
//       * It's rather inefficient, since we write every bit individually
template <typename OutputIterator>
class bitstream_writer final
{
public:
    bitstream_writer(const bitstream_writer&) = delete;
    bitstream_writer& operator=(const bitstream_writer&) = delete;

    explicit bitstream_writer(unbounded_byte_writer<OutputIterator>& byte_writer)
        : m_byte_writer(byte_writer)
    {}

    // TODO: review (data compression book?)
    void write_bit(bool bit)
    {
        if (bit)
        {
            m_bitbuffer |= m_bitmask;
        }

        m_bitmask >>= 1;
        if (!m_bitmask)
        {
            m_bitmask = 0x80000000;
            write32(m_byte_writer, m_bitbuffer);
            m_bitbuffer = 0; // TODO: is this needed? Probably yes, no?
        }
    }

    // TODO: review (data compression book?)
    // TODO: this and write_bit are horribly inefficient
    //       * Can we not write this such that it writes the entire code at once unless it does not fit?
    //       * Do we even use write_bit?
    void write_code(code c, code_length l)
    {
        code mask = 1 << (l - 1); // TODO: this breaks horribly if l is < 0. Do we care? (Can it even happen? Is not code_length unsigned?)
        while (mask)
        {
            write_bit(c & mask);
            mask >>= 1;
        }
    }

    void flush_if_not_empty()
    {
        if (!empty())
        {
            // TODO: copypasted from above
            m_bitmask = 0x80000000;
            write32(m_byte_writer, m_bitbuffer);
            m_bitbuffer = 0; // TODO: is this needed? Probably yes, no?
        }
    }

private:
    bool empty() const
    {
        return m_bitmask == 0x80000000;
    }

    std::uint32_t m_bitbuffer = 0;
    std::uint32_t m_bitmask = 0x80000000;
    unbounded_byte_writer<OutputIterator>& m_byte_writer;
};

template <std::input_iterator InputIterator>
class huffman_decoder_tree final
{
public:
    explicit huffman_decoder_tree(unsigned int symbol_size, byte_reader<InputIterator>& reader)
        : m_symbol_size(symbol_size)
        , m_symbol_max_value((1 << symbol_size) - 1u)
    {
        read_tree(reader);
    }

    agbpack_u8 decode_symbol(bitstream_reader<InputIterator>& bit_reader) const
    {
        bool character_found = false;
        std::size_t current_node_index = 0;
        auto current_node_value = read_tree_node(root_node_index);

        while (!character_found)
        {
            current_node_index += 2u * ((current_node_value & mask_next_node_offset) + 1);

            if (!bit_reader.read_bit())
            {
                character_found = current_node_value & mask0;
                current_node_value = read_tree_node(current_node_index);
            }
            else
            {
                character_found = current_node_value & mask1;
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
        assert((2 <= tree_size) && (tree_size <= max_serialized_tree_size) && "huffman_decoder is broken");

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
    unsigned int m_symbol_size;
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
            // TODO: do we want to test for exactly this case? (we have a test, but should it verify the error message?)
            // TODO: are there more decode_exceptions where we'd want this?
            throw decode_exception("bitstream is misaligned");
        }
    }
};

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
                auto sym = byte & symbol_mask;
                ++m_frequencies[sym];
                byte >>= m_symbol_size;
            }
        }

        return data;
    }

    symbol_frequency frequency(symbol s) const
    {
        assert_symbol(s, m_frequencies);
        return m_frequencies[s];
    }

private:
    unsigned int m_symbol_size;
    std::vector<symbol_frequency> m_frequencies;
};

class tree_node;
using tree_node_ptr = std::shared_ptr<tree_node>;

class tree_node final
{
public:
    explicit tree_node(symbol sym, symbol_frequency frequency) noexcept
        : m_is_leaf(true)
        , m_symbol(sym)
        , m_frequency(frequency)
    {}

    explicit tree_node(tree_node_ptr child0, tree_node_ptr child1)
        : m_is_leaf(false)
        , m_symbol(0)
        , m_frequency(child0->frequency() + child1->frequency())
        , m_child0(child0)
        , m_child1(child1)
    {}

    bool is_leaf() const { return m_is_leaf; }

    symbol sym() const { return m_symbol; }

    symbol_frequency frequency() const { return m_frequency; }

    tree_node_ptr child0() const { return m_child0; }

    tree_node_ptr child1() const { return m_child1; }

    static tree_node_ptr make_leaf(symbol sym, symbol_frequency frequency)
    {
        return std::make_shared<tree_node>(sym, frequency);
    }

    static tree_node_ptr make_internal(tree_node_ptr child0, tree_node_ptr child1)
    {
        return std::make_shared<tree_node>(child0, child1);
    }

private:
    bool m_is_leaf;
    symbol m_symbol;
    symbol_frequency m_frequency;
    tree_node_ptr m_child0;
    tree_node_ptr m_child1;
};

class tree_node_compare final
{
public:
    bool operator()(tree_node_ptr a, tree_node_ptr b)
    {
        return a->frequency() > b->frequency();
    }
};

class huffman_encoder_tree final
{
public:
    explicit huffman_encoder_tree(unsigned int symbol_size, const frequency_table& ftable)
        : m_symbol_size(symbol_size)
        , m_root(build_tree(symbol_size, ftable))
    {}

    tree_node_ptr root() const
    {
        return m_root;
    }

    // TODO: what is the maximum code length for a symbol that we can handle, and can we detect overflows?
    code_table create_code_table() const
    {
        code_table table(m_symbol_size);
        create_code_table_internal(table, m_root.get(), 0, 0);
        return table;
    }

private:
    using node_queue = std::priority_queue<
        tree_node_ptr,
        std::vector<tree_node_ptr>,
        tree_node_compare>;

    static tree_node_ptr build_tree(unsigned int symbol_size, const frequency_table& ftable)
    {
        node_queue nodes;

        // Create a leaf node for each symbol whose frequency is > 0
        auto nsymbols = get_nsymbols(symbol_size);
        for (symbol sym = 0; sym < nsymbols; ++sym)
        {
            symbol_frequency f = ftable.frequency(sym);
            if (f > 0)
            {
                nodes.push(tree_node::make_leaf(sym, f));
            }
        }

        // Both our tree serialization code and the GBA BIOS' huffman tree storage format
        // needs a tree with at least two leaf nodes, even if they're bogus nodes.
        // So add bogus nodes if needed. What makes them bogus is that their frequency is 0.
        // The symbol is irrelevant, but an obvious choice is 0.
        while (nodes.size() < 2) // TODO: this is not yet particularly well tested
        {
            nodes.push(tree_node::make_leaf(0, 0));
        }

        // Standard huffman tree building algorithm:
        // Combine nodes with lowest frequency until there is only one node left: the tree's root node.
        while (nodes.size() > 1)
        {
            auto node0 = pop(nodes);
            auto node1 = pop(nodes);
            nodes.push(tree_node::make_internal(node0, node1));
        }

        assert(nodes.size() == 1);
        auto root = nodes.top();
        return root;
    }

    static tree_node_ptr pop(node_queue& nodes)
    {
        auto node = nodes.top();
        nodes.pop();
        return node;
    }

    static void create_code_table_internal(code_table& table, tree_node* node, code c, code_length l)
    {
        if (node->is_leaf())
        {
            table.set(node->sym(), c, l);
        }
        else
        {
            create_code_table_internal(table, node->child0().get(), c << 1, l + 1);
            create_code_table_internal(table, node->child1().get(), (c << 1) | 1, l + 1);
        }
    }

    unsigned int m_symbol_size;
    tree_node_ptr m_root;
};

// TODO: lots of static casts in here. Reduce to one?
class huffman_tree_serializer final
{
public:
    std::vector<agbpack_u8> serialize(const huffman_encoder_tree& tree)
    {
        auto serialized_tree = create_empty_serialized_tree();
        auto writer = make_unbounded_byte_writer(back_inserter(serialized_tree));

        // Reserve space for tree size byte. We'll fix up its value later.
        writer.write8(0);

        // TODO: document that we do breadth first and why?
        std::queue<tree_node_ptr> queue;
        queue.push(tree.root());
        std::size_t next_index = 1; // TODO: better name/document what this is?

        while (!queue.empty())
        {
            auto node = pop(queue);
            if (!node->is_leaf())
            {
                // TODO: runtime check: value must be in the range..err...what...0..63?

                // Calculate and write internal node value
                auto current_index = writer.nbytes_written() / 2;
                agbpack_u8 internal_node_value = calculate_internal_node_value(node, current_index, next_index);
                writer.write8(internal_node_value);

                // Schedule child nodes for serialization and allocate next slot
                queue.push(node->child0());
                queue.push(node->child1());
                ++next_index;
            }
            else
            {
                // Write leaf node value
                agbpack_u8 leaf_node_value = static_cast<agbpack_u8>(node->sym());
                writer.write8(leaf_node_value);
            }
        }

        write_padding_bytes(writer);

        // Fix up tree size byte
        // TODO: fix up tree size byte (do we need a test for this?) (well we'll automatically have some, no?)
        //       * We just must make sure we have at least one test requiring padding and one requiring no padding
        // TODO: do we need any assertions here?
        //       * Well the tree size should not be greater than 512 bytes
        //       * What about the minimum size?
        //       * The tree size should be a multiple of 4 bytes
        serialized_tree[0] = static_cast<agbpack_u8>(writer.nbytes_written() / 2 - 1);

        // TODO: assert the tree has the right size? It should be 2 * NumNodes + OneByteForStreeSize + Padding (do we really need an assert for that? Just don't forget the padding, no?)
        return serialized_tree;
    }

private:
    std::vector<agbpack_u8> create_empty_serialized_tree()
    {
        std::vector<agbpack_u8> tree;
        tree.reserve(max_serialized_tree_size);
        return tree;
    }

    // Work around CTAD warnings from Clang. Not sure whether these should be enabled.
    template <typename OutputIterator>
    unbounded_byte_writer<OutputIterator> make_unbounded_byte_writer(OutputIterator iterator)
    {
        return unbounded_byte_writer<OutputIterator>(iterator);
    }

    static agbpack_u8 calculate_internal_node_value(tree_node_ptr node, std::size_t current_index, std::size_t next_index)
    {
        std::size_t offset = next_index - current_index - 1;
        // TODO: check range with in_range, then throw
        //       * Problem: in_range does not work well/as expected
        //       * We should maybe have some sort of create_iternal_exception function?

        agbpack_u8 internal_node_value = static_cast<agbpack_u8>(offset);

        if (node->child0()->is_leaf())
        {
            internal_node_value |= mask0;
        }

        if (node->child1()->is_leaf())
        {
            internal_node_value |= mask1;
        }

        return internal_node_value;
    }

    static tree_node_ptr pop(std::queue<tree_node_ptr>& queue)
    {
        auto node = queue.front();
        queue.pop();
        return node;
    }
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
        const auto uncompressed_data = ftable.update(input, eof);

        // Create the tree for the encoder.
        // Also create the serialized variant of the tree and the code table for the encoder.
        huffman_encoder_tree tree(symbol_size, ftable);
        huffman_tree_serializer serializer;
        const auto serialized_tree = serializer.serialize(tree);
        const auto code_table = tree.create_code_table();

        // TODO: bad: static cast
        // TODO: check uncompressed size and throw appropriate exception if too big
        auto header = header::create(m_options, static_cast<uint32_t>(uncompressed_data.size()));

        // Copy header and serialized tree to output, then encode data directly to output.
        unbounded_byte_writer<OutputIterator> writer(output);
        write32(writer, header.to_uint32_t());
        write(writer, serialized_tree.begin(), serialized_tree.end());
        encode_internal(code_table, uncompressed_data, writer);
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
    template <typename OutputIterator>
    static void encode_internal(
        const code_table& code_table,
        const std::vector<agbpack_u8>& uncompressed_data,
        unbounded_byte_writer<OutputIterator>& writer)
    {
        // TODO: do we need this if? not really, no?
        if (!uncompressed_data.size())
        {
            return;
        }

        auto symbol_size = code_table.symbol_size();
        auto symbol_mask = get_symbol_mask(symbol_size);
        bitstream_writer<OutputIterator> bit_writer(writer);

        for (auto byte : uncompressed_data)
        {
            for (unsigned int nbits = 0; nbits < 8; nbits += symbol_size)
            {
                auto sym = byte & symbol_mask;
                bit_writer.write_code(code_table[sym].c(), code_table[sym].l());
                byte >>= symbol_size;
            }
        }
        bit_writer.flush_if_not_empty();
    }

    huffman_options m_options = huffman_options::h8;
};

}

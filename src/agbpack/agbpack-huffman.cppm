// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <format> // TODO: remove
#include <iostream> // TODO: remove
#include <iterator>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string> // TODO: remove
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

// TODO: for data encoding we still need some way to get an entry (by symbol, I guess)
class code_table final
{
public:
    code_table(unsigned int symbol_size) : m_table(get_nsymbols(symbol_size)) {}

    void set(symbol s, code c, code_length l)
    {
        m_table[s] = code_table_entry(s, c, l);
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

            if (!bit_reader.get_bit())
            {
                character_found = current_node_value & mask_0;
                current_node_value = read_tree_node(current_node_index);
            }
            else
            {
                character_found = current_node_value & mask_1;
                current_node_value = read_tree_node(current_node_index + 1);
            }
        }

        if (current_node_value > m_symbol_max_value)
        {
            throw decode_exception();
        }

        return current_node_value;
    }

    code_table create_code_table() const
    {
        // Recursively create a code table from the decoder tree.
        // This is basically the loop from decode_symbol converted into a recursive function,
        // so you might want to refer to that function.
        code_table table(m_symbol_size);
        create_code_table_internal(table, 0, read_tree_node(root_node_index), false, 0, 0);
        return table;
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

    void create_code_table_internal(
        code_table& table,
        std::size_t node_index,
        agbpack_u8 node_value,
        bool is_leaf,
        code c,
        code_length l) const
    {
        if (is_leaf)
        {
            // TODO: take care: if node_value contains garbage in its upper bits we'll have an array overflow
            //       Actually that's why huffman.bad.4.garbage-in-unused-bits-of-leaf-node.txt.encoded fires a debug assertion in the runtime with MSVC
            table.set(node_value, c, l);
        }
        else
        {
            node_index += 2u * ((node_value & mask_next_node_offset) + 1);
            create_code_table_internal(table, node_index, read_tree_node(node_index), node_value & mask_0, c << 1, l + 1);
            create_code_table_internal(table, node_index, read_tree_node(node_index + 1), node_value & mask_1, (c << 1) | 1, l + 1);
        }
    }

    auto read_tree_node(std::size_t node_index) const
    {
        if ((node_index < root_node_index) || (node_index >= m_tree.size()))
        {
            throw decode_exception();
        }

        return m_tree[node_index];
    }

    static constexpr auto mask_0 = 0x80;
    static constexpr auto mask_1 = 0x40;
    static constexpr auto mask_next_node_offset = 63;
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
            throw decode_exception();
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

    symbol_frequency frequency(std::size_t sym) const
    {
        return m_frequencies[sym];
    }

private:
    unsigned int m_symbol_size;
    std::vector<symbol_frequency> m_frequencies;
};

// TODO: document why shared_ptr and not unique_ptr?
//       => Maybe but more importantly factor out the pointer type somehow?
class tree_node final
{
public:
    explicit tree_node(symbol sym, symbol_frequency frequency) noexcept // TODO: do we really want/need this to be noexcept? (recheck)
        : m_is_leaf(true)
        , m_symbol(sym)
        , m_frequency(frequency)
    {}

    // TODO: review
    explicit tree_node(std::shared_ptr<tree_node> child0, std::shared_ptr<tree_node> child1)
        : m_is_leaf(false)
        , m_symbol(0)
        , m_frequency(child0->frequency() + child1->frequency())
        , m_child0(child0)
        , m_child1(child1)
    {}

    bool is_leaf() const { return m_is_leaf; }

    symbol sym() const { return m_symbol; }

    symbol_frequency frequency() const { return m_frequency; }

    std::shared_ptr<tree_node> child0() const { return m_child0; }

    std::shared_ptr<tree_node> child1() const { return m_child1; }

private:
    bool m_is_leaf;
    symbol m_symbol;
    symbol_frequency m_frequency;
    std::shared_ptr<tree_node> m_child0;
    std::shared_ptr<tree_node> m_child1;
};

class tree_node_compare final
{
public:
    bool operator()(std::shared_ptr<tree_node> a, std::shared_ptr<tree_node> b)
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

    code_table create_code_table() const
    {
        code_table table(m_symbol_size);
        create_code_table_internal(table, m_root, 0, 0);
        return table;
    }

    // TODO: make private
    // TODO: use raw pointers rather than shared pointers?
    void create_code_table_internal(code_table& table, std::shared_ptr<tree_node> node, code c, code_length l) const
    {
        if (node->is_leaf())
        {
            table.set(node->sym(), c, l);
        }
        else
        {
            // TODO: update code
            create_code_table_internal(table, node->child0(), c, l + 1);
            create_code_table_internal(table, node->child1(), c, l + 1);
        }
    }

    // TODO: remove, respectively replace this by a create_code_table function
    void dump()
    {
        dump_internal(m_root, "");
    }

    // TODO: remove/make private
    void dump_internal(std::shared_ptr<tree_node> node, std::string ccode)
    {
        if (node->is_leaf())
        {
            std::cout << std::format("{}: {}\n", static_cast<char>(node->sym()), ccode);
        }
        else
        {
            dump_internal(node->child0(), ccode + "0");
            dump_internal(node->child1(), ccode + "1");
        }
    }

private:
    using node_queue = std::priority_queue<
        std::shared_ptr<tree_node>,
        std::vector<std::shared_ptr<tree_node>>,
        tree_node_compare>;

    static std::shared_ptr<tree_node> build_tree(unsigned int symbol_size, const frequency_table& ftable)
    {
        node_queue nodes;

        // Create a leaf node for each symbol whose frequency is > 0
        auto nsymbols = get_nsymbols(symbol_size);
        for (symbol sym = 0; sym < nsymbols; ++sym)
        {
            symbol_frequency f = ftable.frequency(sym);
            if (f > 0)
            {
                nodes.push(std::make_shared<tree_node>(sym, f));
            }
        }

        // Both our tree serialization code and the GBA BIOS' huffman tree storage format
        // needs a tree with at least two leaf nodes, even if they're bogus nodes.
        // So add bogus nodes if needed. What makes them bogus is that their frequency is 0.
        // The symbol is irrelevant, but an obvious choice is 0.
        while (nodes.size() < 2) // TODO: this is not yet particularly well tested
        {
            // TODO: out of curiosity: which ctor gets called?
            //       * obviously we mean the one for leaf nodes, but it's very well possibly the one for intermediate notes gets called
            //       * Well we can have factory methods instead of constructors, then things will be clear too.
            //         * Uh well no not really, make_shared does not work with private constructors. Not without jumping through hoops, anyway
            nodes.push(std::make_shared<tree_node>(0, 0));
        }

        // Standard huffman tree building algorithm:
        // Combine nodes with lowest frequency until there is only one node left: the tree's root node.
        while (nodes.size() > 1)
        {
            auto node0 = nodes.top();
            nodes.pop();
            auto node1 = nodes.top();
            nodes.pop();
            nodes.push(std::make_shared<tree_node>(node0, node1));
        }

        assert(nodes.size() == 1);
        auto root = nodes.top();
        return root;
    }

    unsigned int m_symbol_size;
    std::shared_ptr<tree_node> m_root;
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

        // TODO: testcode: remove -----------------------
        const auto code_table = tree.create_code_table();
        code_table.dump();
        // ----------------------------------------------

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

// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <format>
#include <iostream>
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

class code_table final
{
public:
    code_table(unsigned int symbol_size) : m_table(get_nsymbols(symbol_size)) {}

    void set(symbol s, code c, code_length l)
    {
        // TODO: assert index
        m_table[s] = code_table_entry(s, c, l);
    }

    // TODO: review signature
    const code_table_entry& get(symbol s) const
    {
        // TODO: assert index
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

private:
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
        // TODO: assert index
        return m_frequencies[sym];
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

    // TODO: recheck tree generation, since we changed it so much
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

class huffman_tree_serializer final
{
public:
    // TODO: figure out public interface
    // TODO: implement:
    //       * Naive implementation first? => Recursive?
    //       * Somehow navigate the tree
    //         * For each node of the tree
    //           * Figure out where to put its children into the array
    //           * Write its children into the array
    std::vector<agbpack_u8> serialize(const huffman_encoder_tree& tree)
    {
        // TODO: return something. For starters, the hardcoded data from below will do.
        // TODO: do something here.
        //       * Basically, traverse the tree somehow [Umm...how? For starters, recursively? Or do we go for depth first right away?
        //         * For each node, figure out where to put its children, and store that information along with the children (orly?)
        //           * Note: The root node is always at the beginning of the aray.
        //           * Store the child type bits in the node
        //           * Store the offset in the node

        // TODO: remove
        std::cout << "----------\n";

        // TODO: document that we do breadth first and why?
        std::queue<tree_node_ptr> queue;
        queue.push(tree.root());

        while (!queue.empty())
        {
            // TODO: ugh: actually, serialization of an intermediate is very different from a leaf node:
            //            * For the leaf node we simply write out the symbol()
            //            * Well this means we DO need an if/else statement, no?

            // TODO: do something with the current node:
            //       * Serialize it:
            //         * Need to know:
            //           * child0 type
            //           * child1 type
            //           * Offset => When we assign the offset, do not forget it has the right range. What is it? 0..63 or what?
            //       * Do we need to special handle anything?
            //         * tree size byte?
            //         * root node?
            auto node = pop(queue);
            if (!node->is_leaf())
            {
                std::cout << "<internal node>\n"; // TODO: remove
                queue.push(node->child0());
                queue.push(node->child1());
            }
            else
            {
                // TODO: remove
                std::cout << static_cast<char>(node->sym()) << "\n";
            }
        }

        // TODO: actually return serialized tree
        // TODO: assert the tree has the right size? It should be 2 * NumNodes + OneByteForStreeSize + Padding (do we really need an assert for that? Just don't forget the padding, no?)
        return {};
    }

private:
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

        // Create the tree for the encoder. Also create the serialized variant of the tree.
        huffman_encoder_tree tree(symbol_size, ftable);
        huffman_tree_serializer serializer;
        serializer.serialize(tree);

        // --- TODO: remove: hardcoded huffman tree data from CUE ---
        std::vector<unsigned char> hardcoded_tree_data;
        if (uncompressed_data.size() == 0)
        {
            hardcoded_tree_data = { 0x01, 0xc0, 0x00, 0x00 };
        }
        else
        {
            // From: huffman.good.8.helloworld.txt
            hardcoded_tree_data = { 0x07, 0x00, 0x00, 0x81, 0xc1, 0xc2, 0x6c, 0x82, 0x64, 0x65, 0x72, 0x77, 0x6f, 0xc0, 0x20, 0x48 };
        }
        byte_reader<std::vector<unsigned char>::iterator> br(hardcoded_tree_data.begin(), hardcoded_tree_data.end());
        huffman_decoder_tree<std::vector<unsigned char>::iterator> dt(symbol_size, br);
        // ----------------------------------------------------------
        const auto code_table = dt.create_code_table(); // TODO: obtain code table from tree, not dt (which is our temporary decoder tree)

        // TODO: as usual, need to encode stuff to temporary buffer
        // TODO: actually encode stuff
        //       * serialize tree to output
        //       * encode data to output

        // TODO: bad: static cast
        // TODO: check uncompressed size and throw appropriate exception if too big
        auto header = header::create(m_options, static_cast<uint32_t>(uncompressed_data.size()));

        // Copy header and encoded data to output
        unbounded_byte_writer<OutputIterator> writer(output);
        write32(writer, header.to_uint32_t());

        // TODO: unhardcode tree data
        // TODO: ensure tree data is correctly padded (it DOES need padding, right? Can we test this somehow?)
        write(writer, hardcoded_tree_data.begin(), hardcoded_tree_data.end());

        // TODO: write encoded data (ensure correct alignment!)
        if (uncompressed_data.size())
        {
            bitstream_writer<OutputIterator> bit_writer(writer);
            for (auto byte : uncompressed_data)
            {
                // TODO: only half the truth: one byte does not equal one symbol
                //       => This does not yet work for 4 bit huffman!
                // TODO: use an indexing operator for code_table?
                const auto& e = code_table.get(byte);
                bit_writer.write_code(e.c(), e.l());
            }
            // TODO: loop over input bytes (and then later symbols of input bytes in the case of 4 bit huffman(
            //       * For each byte
            //         * Extract all the symbols (one or 2)
            //           * For each symbol:
            //             * Get code and length from code table
            //             * Write to bit_writer
            // TODO: need a way to flush the input buffer (when uncompressed data is EOF and there are still bits in the bit buffer)
        }
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

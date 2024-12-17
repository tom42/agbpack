// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <iterator>
#include <map>
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

using size_t = std::size_t;

AGBPACK_EXPORT_FOR_UNIT_TESTING using symbol = unsigned int; // TODO: this should not be agbpack_u8. Document this / write test that would figure out?
AGBPACK_EXPORT_FOR_UNIT_TESTING using symbol_frequency = uint32_t;
using code = uint32_t;
using code_length = unsigned int;

inline constexpr auto root_node_index = 1;
inline constexpr auto min_next_node_offset = 0u;
inline constexpr auto max_next_node_offset = 63u;
inline constexpr auto mask_next_node_offset = 63;
inline constexpr auto min_encoded_tree_size = 4u;
inline constexpr auto max_encoded_tree_size = 512u;
inline constexpr auto mask0 = 0x80;
inline constexpr auto mask1 = 0x40;
AGBPACK_EXPORT_FOR_UNIT_TESTING inline constexpr auto max_code_length = std::numeric_limits<code>::digits;

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

bool in_closed_range(std::unsigned_integral auto x, std::unsigned_integral auto min, std::unsigned_integral auto max)
{
    return (min <= x) && (x <= max);
}

bool in_open_range(std::unsigned_integral auto x, std::unsigned_integral auto min, std::unsigned_integral auto max)
{
    return (min <= x) && (x < max);
}

template <typename TContainer>
void assert_symbol([[maybe_unused]] symbol s, [[maybe_unused]] const TContainer& container)
{
    assert(in_open_range(s, 0u, container.size()) && "symbol value is out of range");
}

AGBPACK_EXPORT_FOR_UNIT_TESTING
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

    bool operator==(const code_table_entry&) const = default;

private:
    symbol m_s;
    code m_c;
    code_length m_l;
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
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

private:
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

AGBPACK_EXPORT_FOR_UNIT_TESTING
template <typename OutputIterator>
class bitstream_writer final
{
public:
    bitstream_writer(const bitstream_writer&) = delete;
    bitstream_writer& operator=(const bitstream_writer&) = delete;

    explicit bitstream_writer(unbounded_byte_writer<OutputIterator>& byte_writer)
        : m_byte_writer(byte_writer)
    {
        reset();
    }

    void write_code(code c, code_length l)
    {
        assert(l > 0);

        code mask = 1 << (l - 1);
        while (mask)
        {
            write_bit(c & mask);
            mask >>= 1;
        }
    }

    void flush()
    {
        if (!empty())
        {
            write32(m_byte_writer, m_bitbuffer);
            reset();
        }
    }

private:
    void reset()
    {
        m_bitbuffer = 0;
        m_bitmask = initial_bitmask;
    }

    void write_bit(bool bit)
    {
        if (bit)
        {
            m_bitbuffer |= m_bitmask;
        }

        m_bitmask >>= 1;

        if (full())
        {
            flush();
        }
    }

    bool empty() const
    {
        return m_bitmask == initial_bitmask;
    }

    bool full() const
    {
        return m_bitmask == 0;
    }

    static constexpr std::uint32_t initial_bitmask = 0x80000000;
    unbounded_byte_writer<OutputIterator>& m_byte_writer;
    std::uint32_t m_bitbuffer;
    std::uint32_t m_bitmask;
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
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
        size_t current_node_index = 0;
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

        throw_if_decoded_symbol_is_invalid(current_node_value);

        return current_node_value;
    }

    code_table create_code_table() const
    {
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
        size_t tree_size = 2u * (read8(reader) + 1);

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
        size_t node_index,
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
            create_code_table_internal(table, node_index, read_tree_node(node_index), node_value & mask0, c << 1, l + 1);
            create_code_table_internal(table, node_index, read_tree_node(node_index + 1), node_value & mask1, (c << 1) | 1, l + 1);
        }
    }

    agbpack_u8 read_tree_node(size_t node_index) const
    {
        if ((node_index < root_node_index) || (node_index >= m_tree.size()))
        {
            throw decode_exception();
        }

        return m_tree[node_index];
    }

    void throw_if_decoded_symbol_is_invalid(agbpack_u8 symbol) const
    {
        if (symbol > m_symbol_max_value)
        {
            throw decode_exception();
        }
    }

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
            throw decode_exception("bitstream is misaligned");
        }
    }
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
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

    void set_frequency(symbol s, symbol_frequency f)
    {
        assert_symbol(s, m_frequencies);
        m_frequencies[s] = f;
    }

private:
    unsigned int m_symbol_size;
    std::vector<symbol_frequency> m_frequencies;
};

AGBPACK_EXPORT_FOR_UNIT_TESTING class huffman_tree_node;
using huffman_tree_node_ptr = std::unique_ptr<huffman_tree_node>;

// TODO: review/rework very thoroughly
// TODO: do we really have to have mutable nodes for serialization?
AGBPACK_EXPORT_FOR_UNIT_TESTING
class huffman_tree_node final
{
public:
    huffman_tree_node(symbol sym, symbol_frequency frequency)
        : m_frequency(frequency)
        , m_sym(sym)
    {}

    huffman_tree_node(huffman_tree_node_ptr left, huffman_tree_node_ptr right)
        : m_children{ std::move(left), std::move(right) }, m_frequency(m_children[0]->m_frequency + m_children[1]->m_frequency)
    {}

    huffman_tree_node() = delete;

    huffman_tree_node(const huffman_tree_node& other) = delete;

    huffman_tree_node(huffman_tree_node&& other) = delete;

    huffman_tree_node& operator=(const huffman_tree_node& other) = delete;

    huffman_tree_node& operator=(huffman_tree_node&& other) = delete;

    bool is_internal() const
    {
        return m_children[0] != nullptr;
    }

    // Returns the number of nodes in this subtree
    size_t num_nodes() const
    {
        if (is_internal())
        {
            // Sum of children plus self
            return m_children[0]->num_nodes() + m_children[1]->num_nodes() + 1;
        }

        // This is a data node, just count self
        return 1;
    }

    // Returns the number of leaves in this subtree
    size_t num_leaves() const
    {
        if (m_leaves == 0)
        {
            if (is_internal())
            {
                m_leaves = m_children[0]->num_leaves() + m_children[1]->num_leaves();
            }
            else
            {
                m_leaves = 1;
            }
        }

        return m_leaves;
    }

    // Returns the symbol represented by a leaf node. No meaning for internal nodes.
    symbol sym() const
    {
        return m_sym;
    }

    symbol_frequency frequency() const
    {
        return m_frequency;
    }

    const huffman_tree_node_ptr& child(size_t index) const
    {
        assert((index == 0) || (index == 1));
        return m_children[index];
    }

    static huffman_tree_node_ptr make_leaf(symbol sym, symbol_frequency frequency)
    {
        return std::make_unique<huffman_tree_node>(sym, frequency);
    }

    static huffman_tree_node_ptr make_internal(huffman_tree_node_ptr child0, huffman_tree_node_ptr child1)
    {
        return std::make_unique<huffman_tree_node>(std::move(child0), std::move(child1));
    }

private:
    std::array<huffman_tree_node_ptr, 2> m_children{};
    symbol_frequency m_frequency = 0;
    symbol m_sym = 0;
    mutable size_t m_leaves = 0;

public: // TODO: this is temporarily public
    // TODO: I'd prefer if tree_node was immutable - however, we currently need m_val to be writable.
    //       This is really silly: 'representing a tree' and 'serializing a tree' are two different things, and m_val is required only for the latter
    size_t m_offset = 0;
};

class tree_node_compare final
{
public:
    // TODO: in principle we want a and b be constant. How to achieve? Also fix this in other bits of the code
    bool operator()(const huffman_tree_node_ptr& a, const huffman_tree_node_ptr& b)
    {
        if (a->frequency() != b->frequency())
        {
            return a->frequency() > b->frequency();
        }

        return a->sym() > b->sym();
    }
};

// Replacement for std::priority_queue that works with std::unique_ptr.
// As a bonus it also supports reserve().
AGBPACK_EXPORT_FOR_UNIT_TESTING
class node_priority_queue final
{
public:
    void reserve(size_t capacity)
    {
        m_queue.reserve(capacity);
    }

    size_t size() const
    {
        return m_queue.size();
    }

    void push(huffman_tree_node_ptr node)
    {
        m_queue.push_back(std::move(node));
        std::ranges::push_heap(m_queue, tree_node_compare());
    }

    huffman_tree_node_ptr pop()
    {
        std::ranges::pop_heap(m_queue, tree_node_compare());
        auto node = std::move(m_queue.back());
        m_queue.pop_back();
        return node;
    }

private:
    std::vector<huffman_tree_node_ptr> m_queue;
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
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
        create_code_table_internal(table, m_root.get(), 0, 0);
        return table;
    }

    const huffman_tree_node_ptr& root() const
    {
        return m_root;
    }

private:
    static huffman_tree_node_ptr build_tree(unsigned int symbol_size, const frequency_table& ftable)
    {
        auto nodes = create_leaf_nodes(symbol_size, ftable);
        auto root = combine_nodes(nodes);
        return root;
    }

    static node_priority_queue create_leaf_nodes(unsigned int symbol_size, const frequency_table& ftable)
    {
        node_priority_queue nodes;

        // Create a leaf node for each symbol whose frequency is > 0
        auto nsymbols = get_nsymbols(symbol_size);
        for (symbol sym = 0; sym < nsymbols; ++sym)
        {
            symbol_frequency f = ftable.frequency(sym);
            if (f > 0)
            {
                nodes.push(huffman_tree_node::make_leaf(sym, f));
            }
        }

        // Both our tree serialization code and the GBA BIOS' huffman tree storage format
        // need a tree with at least two leaf nodes, even if they're bogus nodes.
        // So add bogus nodes if needed. What makes them bogus is that their frequency is 0.
        // The symbol is irrelevant, so we use 0.
        while (nodes.size() < 2)
        {
            nodes.push(huffman_tree_node::make_leaf(0, 0));
        }

        return nodes;
    }

    static huffman_tree_node_ptr combine_nodes(node_priority_queue& nodes)
    {
        // Standard huffman tree building algorithm:
        // Combine nodes with lowest frequency until there is only one node left: the tree's root node.
        while (nodes.size() > 1)
        {
            auto node0 = nodes.pop();
            auto node1 = nodes.pop();
            nodes.push(huffman_tree_node::make_internal(std::move(node0), std::move(node1)));
        }

        return nodes.pop();
    }

    static void create_code_table_internal(code_table& table, huffman_tree_node* node, code c, code_length l)
    {
        if (l > max_code_length)
        {
            throw encode_exception("maximum code length exceeded");
        }

        if (node->is_internal())
        {
            create_code_table_internal(table, node->child(0).get(), c << 1, l + 1);
            create_code_table_internal(table, node->child(1).get(), (c << 1) | 1, l + 1);
        }
        else
        {
            table.set(node->sym(), c, l);
        }
    }

    unsigned int m_symbol_size;
    huffman_tree_node_ptr m_root;
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
class huffman_tree_serializer final
{
public:
    std::vector<agbpack_u8> serialize(const huffman_encoder_tree& tree)
    {
        auto serialized = create_empty_serialized_tree(tree);

        serialized[root_node_index] = tree.root().get();

        serialize_tree(serialized, tree.root().get(), root_node_index + 1);
        fixup_tree(serialized);
        assert_tree(serialized);
        return encode_tree(serialized);
    }

private:
    using serialized_tree = std::vector<huffman_tree_node*>;

    static serialized_tree create_empty_serialized_tree(const huffman_encoder_tree& tree)
    {
        // Allocate space for all internal and leaf nodes.
        // Also allocate an extra slot for the tree size byte. We don't store anything there in the
        // serialized tree, but it is helpful if the root node occupies the array element at index 1.
        serialized_tree serialized_tree(tree.root()->num_nodes() + 1);
        return serialized_tree;
    }

    static void serialize_tree(serialized_tree& tree, huffman_tree_node* node, size_t next)
    {
        // TODO: review very thoroughly
        //       * Unhardcode 0x40
        //       * Array indices should be of type size_t
        assert(node->is_internal());

        if (node->num_leaves() > 0x40)
        {
            // This subtree will overflow the offset field if inserted naively
            tree[next + 0] = node->child(0).get();
            tree[next + 1] = node->child(1).get();

            unsigned a = 0;
            unsigned b = 1;

            if (node->child(1)->num_leaves() < node->child(0)->num_leaves())
            {
                std::swap(a, b);
            }

            if (node->child(a)->is_internal())
            {
                node->child(a)->m_offset = 0;
                serialize_tree(tree, node->child(a).get(), next + 2);
            }

            if (node->child(b)->is_internal())
            {
                node->child(b)->m_offset = node->child(a)->num_leaves() - 1;
                serialize_tree(tree, node->child(b).get(), next + 2 * node->child(a)->num_leaves());
            }

            return;
        }

        std::deque<huffman_tree_node*> queue;

        queue.emplace_back(node->child(0).get());
        queue.emplace_back(node->child(1).get());

        while (!queue.empty())
        {
            node = queue.front();
            queue.pop_front();

            tree[next++] = node;

            if (!node->is_internal())
            {
                continue;
            }

            node->m_offset = queue.size() / 2;

            queue.emplace_back(node->child(0).get());
            queue.emplace_back(node->child(1).get());
        }
    }

    static void fixup_tree(serialized_tree& tree)
    {
        for (size_t i = root_node_index; i < tree.size(); ++i)
        {
            if (!tree[i]->is_internal() || tree[i]->m_offset <= max_next_node_offset)
            {
                continue;
            }

            size_t shift = tree[i]->m_offset - max_next_node_offset;

            if ((i & 1) && tree[i - 1]->m_offset == max_next_node_offset)
            {
                // Right child, and left sibling would overflow if we shifted;
                // Shift the left child by 1 instead
                --i;
                shift = 1;
            }

            const size_t node_end = i / 2 + 1 + tree[i]->m_offset;
            const size_t node_begin = node_end - shift;

            const size_t shift_begin = 2 * node_begin;
            const size_t shift_end = 2 * node_end;

            // Move last child pair to front
            std::ranges::rotate(&tree[shift_begin], &tree[shift_end], &tree[shift_end + 2]);

            // Adjust offsets
            tree[i]->m_offset -= shift;
            for (size_t index = i + 1; index < shift_begin; ++index)
            {
                if (!tree[index]->is_internal())
                {
                    continue;
                }

                size_t node = index / 2 + 1 + tree[index]->m_offset;
                if (node >= node_begin && node < node_end)
                {
                    ++tree[index]->m_offset;
                }
            }

            if (tree[shift_begin + 0]->is_internal())
            {
                tree[shift_begin + 0]->m_offset += shift;
            }
            if (tree[shift_begin + 1]->is_internal())
            {
                tree[shift_begin + 1]->m_offset += shift;
            }

            for (size_t index = shift_begin + 2; index < shift_end + 2; ++index)
            {
                if (!tree[index]->is_internal())
                {
                    continue;
                }

                size_t node = index / 2 + 1 + tree[index]->m_offset;
                if (node > node_end)
                {
                    --tree[index]->m_offset;
                }
            }
        }
    }

    static void assert_tree([[maybe_unused]] const serialized_tree& serialized_tree)
    {
        // TODO: do we want to have this check always?
#ifndef NDEBUG
        // TODO: name
        // TODO: does map have a capacity?
        // TODO: should we use an unordered map?
        std::map<huffman_tree_node*, size_t> foo;

        for (size_t i = root_node_index; i < serialized_tree.size(); ++i)
        {
            assert(serialized_tree[i]);
            foo[serialized_tree[i]] = i;
        }

        for (size_t i = root_node_index; i < serialized_tree.size(); ++i)
        {
            auto node = serialized_tree[i];
            if (!node->is_internal())
            {
                continue;
            }

            assert(!(node->m_offset & mask0));
            assert(!(node->m_offset & mask1));
            assert(foo[node->child(0).get()] == (foo[node] & ~1u) + 2 * node->m_offset + 2);
        }
#endif
    }

    static std::vector<agbpack_u8> encode_tree(const serialized_tree& serialized_tree)
    {
        auto encoded_tree = create_empty_encoded_tree(serialized_tree);

        // Write tree size byte
        encoded_tree[0] = static_cast<agbpack_u8>(encoded_tree.size() / 2 - 1);

        for (size_t i = root_node_index; i < serialized_tree.size(); ++i)
        {
            encoded_tree[i] = encode_node(serialized_tree[i]);
        }

        assert(in_closed_range(encoded_tree.size(), min_encoded_tree_size, max_encoded_tree_size));
        assert((encoded_tree.size() % 4) == 0);

        return encoded_tree;
    }

    static agbpack_u8 encode_node(const huffman_tree_node* node)
    {
        if (node->is_internal())
        {
            return encode_internal_node(node);
        }
        else
        {
            return encode_leaf_node(node);
        }
    }

    static agbpack_u8 encode_internal_node(const huffman_tree_node* node)
    {
        // TODO: check offset (orly? do we check once more?)
        //       => Well that really should be done inside assert_tree (which should always do it, not only in debug builds)
        //       => Anyway, here's what we had at some point (imo this check should be way before offset is converted to 8 bits)
        //            if (!in_closed_range(offset, min_next_node_offset, max_next_node_offset))
        //            {
        //                throw internal_error("next node offset is out of range");
        //            }

        agbpack_u8 encoded_node = static_cast<agbpack_u8>(node->m_offset);

        if (!node->child(0)->is_internal())
        {
            encoded_node |= mask0;
        }

        if (!node->child(1)->is_internal())
        {
            encoded_node |= mask1;
        }

        return encoded_node;
    }

    static agbpack_u8 encode_leaf_node(const huffman_tree_node* node)
    {
        return static_cast<agbpack_u8>(node->sym());
    }

    static std::vector<agbpack_u8> create_empty_encoded_tree(const serialized_tree& serialized_tree)
    {
        return std::vector<agbpack_u8>(calculate_encoded_tree_size(serialized_tree));
    }

    static size_t calculate_encoded_tree_size(const serialized_tree& serialized_tree)
    {
        // Calculate size of encoded tree.
        // serialized_tree.size() already includes the tree size byte, so we just need to add alignment bytes.
        size_t size = serialized_tree.size();

        while ((size % 4) != 0)
        {
            ++size;
        }

        return size;
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
        auto header = header::create(m_options, static_cast<std::uint32_t>(uncompressed_data.size()));

        // Copy header and tree to output, then encode data directly to output.
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
        bit_writer.flush();
    }

    huffman_options m_options = huffman_options::h8;
};

}

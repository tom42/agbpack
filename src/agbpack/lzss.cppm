// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <utility>
#include <vector>

export module agbpack:lzss;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

// Note: In VS 2022, MSVC for x86 bugs if we fully qualify std::size_t in the lzss_sliding_window class template.
// We work around this by importing it (referring to C's global size_t would probably work too).
using size_t = std::size_t;

inline constexpr size_t minimum_offset = 1;
inline constexpr size_t maximum_offset = 4096;
inline constexpr size_t minimum_vram_safe_offset = 2;
inline constexpr size_t minimum_match_length = 3;
inline constexpr size_t maximum_match_length = 18;

constexpr size_t get_minimum_offset(bool vram_safe)
{
    return vram_safe ? minimum_vram_safe_offset : minimum_offset;
}

// Sliding window for LZSS decoder. Used when the output iterator does not allow random access.
// * Maintains an internal write position which wraps around when the window is written to.
// * Allows reading relative to the write position. The final read position wraps around.
//   Note: the sliding window is not initialized. Reading from a position that has not yet
//   been written to returns uninitialized data.
template <size_t Size>
class lzss_sliding_window final
{
public:
    agbpack_u8 read8(size_t offset)
    {
        assert_read_allowed(offset);
        return m_buf[(m_nbytes_written - offset) & index_mask];
    }

    void write8(agbpack_u8 byte)
    {
        m_buf[m_nbytes_written & index_mask] = byte;
        ++m_nbytes_written;
    }

private:
    static_assert(std::popcount(Size) == 1, "Size must be a power of 2 for index calculations using operator & to work");
    static constexpr size_t index_mask = Size - 1;

    void assert_read_allowed([[maybe_unused]] size_t offset)
    {
        // Required for g++ 14.2.0, which ignores [[maybe_unused]] here
        std::ignore = offset;

        assert((m_nbytes_written > 0) && "Cannot read from empty sliding window");
        assert((offset > 0) && "Cannot read from current write position");
        assert((offset <= Size) && "Offset is too big");
        assert((offset <= m_nbytes_written) && "Offset is too big for amount of data currently in sliding window");
    }

    size_t m_nbytes_written = 0;
    std::array<agbpack_u8, Size> m_buf;
};

// General case LZSS byte writer.
// Works with any kind of output iterator, including those that cannot be read from.
// In order to decode references it maintains an internal sliding window.
template <typename OutputIterator>
class lzss_byte_writer final
{
public:
    explicit lzss_byte_writer(agbpack_u32 uncompressed_size, OutputIterator output)
        : m_writer(uncompressed_size, output) {}

    void write8(agbpack_u8 byte)
    {
        m_writer.write8(byte);
        m_window.write8(byte);
    }

    void copy_from_output(unsigned int nbytes, size_t offset)
    {
        while (nbytes--)
        {
            auto byte = m_window.read8(offset);
            write8(byte);
        }
    }

    bool done() const
    {
        return m_writer.done();
    }

    size_t nbytes_written() const
    {
        return m_writer.nbytes_written();
    }

private:
    byte_writer<OutputIterator> m_writer;
    lzss_sliding_window<maximum_offset> m_window;
};

// Specialized LZSS byte writer for random access iterators.
// Does not need memory for a separate sliding window because references can be read from the output buffer.
template <std::random_access_iterator RandomAccessIterator>
class lzss_byte_writer<RandomAccessIterator> final
{
public:
    explicit lzss_byte_writer(agbpack_u32 uncompressed_size, RandomAccessIterator output)
        : m_uncompressed_size(uncompressed_size)
        , m_output(output)
    {}

    void write8(agbpack_u8 byte)
    {
        if (done())
        {
            throw decode_exception();
        }

        *m_output++ = byte;
        ++m_nbytes_written;
    }

    void copy_from_output(unsigned int nbytes, size_t offset)
    {
        while (nbytes--)
        {
            auto byte = *(m_output - make_signed(offset));
            write8(byte);
        }
    }

    bool done() const
    {
        return m_nbytes_written >= m_uncompressed_size;
    }

    size_t nbytes_written() const
    {
        return m_nbytes_written;
    }

private:
    template <std::unsigned_integral UnsignedIntegral>
    static auto make_signed(UnsignedIntegral value)
    {
        return static_cast<std::make_signed_t<UnsignedIntegral>>(value);
    }

    agbpack_u32 m_uncompressed_size;
    agbpack_u32 m_nbytes_written = 0;
    RandomAccessIterator m_output;
};

export class lzss_decoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type(input); // TODO: probably we want to either remove this or extend it with the output iterator?

        byte_reader<InputIterator> reader(input, eof);
        auto header = header::parse_for_type(compression_type::lzss, read32(reader));
        if (!header)
        {
            throw decode_exception();
        }

        lzss_byte_writer<OutputIterator> writer(header->uncompressed_size(), output);

        unsigned int mask = 0;
        unsigned int flags = 0;

        while (!writer.done())
        {
            mask >>= 1;
            if (!mask)
            {
                flags = read8(reader);
                mask = 0x80;
            }

            if (flags & mask)
            {
                auto b0 = read8(reader);
                auto b1 = read8(reader);
                unsigned int nbytes = ((b0 >> 4) & 0xf) + minimum_match_length;
                size_t offset = (((b0 & 0xfu) << 8) | b1) + minimum_offset;

                assert(in_closed_range(nbytes, minimum_match_length, maximum_match_length) && "lzss_decoder is broken");
                assert(in_closed_range(offset, minimum_offset, maximum_offset) && "lzss_decoder is broken");

                throw_if_not_vram_safe(offset);

                // TODO: tests for invalid input
                //       * read outside of sliding window
                // TODO: must check if this under/overflows!
                //       Note: this CAN happen at runtime when the encoded stream is corrupt, so cannot be just an assert()
                //       * Well what is over. and what is underflow?
                //       * Can we even have both? I don't think so, no?
                //         * The only case we CAN have is when the offset points farther back than the # of bytes in the sliding window, no?
                //       WELL...let's see
                //       * At this point, offset is in range [1, 4096]
                //       * Well in principle, n_bytes_written must be >= offset, no?
                //       * As said, offset is in range [1, 4096]
                //         * If offset must be <= n_bytes_written, then it always fails if no bytes have been written since offset is >= 1
                //       * Unit test this, at least a bit!
                if (offset > writer.nbytes_written())
                {
                    // TODO: dedicated error message? And do we test for it?
                    throw decode_exception();
                }
                writer.copy_from_output(nbytes, offset);
            }
            else
            {
                auto byte = read8(reader);
                writer.write8(byte);
            }
        }

        parse_padding_bytes(reader);
    }

    // When VRAM safety is enabled in the decoder, the decoder throws if the encoded data is not VRAM safe.
    // Use this when you want to verify that data is VRAM safe.
    void vram_safe(bool enable)
    {
        m_vram_safe = enable;
    }

    bool vram_safe() const
    {
        return m_vram_safe;
    }

private:
    void throw_if_not_vram_safe(size_t offset)
    {
        if (offset < get_minimum_offset(m_vram_safe))
        {
            throw decode_exception("encoded data is not VRAM safe");
        }
    }

    bool m_vram_safe = false;
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
class match final
{
public:
    match(size_t length, size_t offset)
        : m_length(length)
        , m_offset(offset)
    {}

    size_t length() const { return m_length; }

    size_t offset() const { return m_offset; }

    bool operator==(const match&) const = default;

private:
    size_t m_length;
    size_t m_offset;
};

// Simple implementation using two nested loops each doing linear search.
AGBPACK_EXPORT_FOR_UNIT_TESTING
class match_finder final
{
public:
    // Note: match_finder does not own input
    explicit match_finder(const std::vector<agbpack_u8>& input, size_t minimum_match_offset)
        : m_input(input)
        , m_minimum_match_offset(minimum_match_offset)
    {}

    // TODO: review this again (compare against reference search)
    match find_match(size_t current_position) const
    {
        match best_match(0, 0); // TODO: start with length=0, or length=2 (minimum_match_length-1)?

        // TODO: implement this.
        //       Basic idea: two nested loops.
        //       * Outer loop: search backwards through sliding window (towards larger offset, away from current position => Nope, wrong, we start at longest offset and keep doing so while it is not 0)
        //         * Inner loop: search forwards, increasing string length.
        //           * Two problems here:
        //             * The maximum match length to search for is of course maximum_match_length, but towards the end of the buffer it may be shorter

        size_t offset = std::min(current_position, maximum_offset);

        for (; offset > m_minimum_match_offset; --offset)
        {
            // TODO: close to end of input it is pointless iterating all over [0, maximum_match_length] if that exceeds input. But is it worth optimizing this?
            size_t length = 0;
            for (; length < maximum_match_length; ++length)
            {
                if (current_position + length >= m_input.size())
                {
                    // End of lookahead reached, abort search
                    break;
                }

                if (m_input[current_position + length] != m_input[current_position + length - offset])
                {
                    // Characters differ, abort search
                    break;
                }
            }

            if (length > best_match.length())
            {
                best_match = match(length, offset);
                if (length >= maximum_match_length)
                {
                    // Found a match of maximum length, no need to search any further.
                    break;
                }
            }
        }

        return best_match;
    }

private:
    const std::vector<agbpack_u8>& m_input;
    size_t m_minimum_match_offset;
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
class lzss_bitstream_writer final
{
public:
    // Note: lzss_bitstream_writer does not own encoded_data
    explicit lzss_bitstream_writer(std::vector<agbpack_u8>& encoded_data)
        : m_encoded_data(encoded_data)
    {}

    void write_literal(agbpack_u8 literal)
    {
        write_tag(false);
        m_encoded_data.push_back(literal);
    }

    void write_reference(size_t length, size_t offset)
    {
        assert(in_closed_range(length, minimum_match_length, maximum_match_length));
        assert(in_closed_range(offset, minimum_offset, maximum_offset));

        write_tag(true);

        auto b0 = ((length - minimum_match_length) << 4) | ((offset - minimum_offset) >> 8);
        auto b1 = (offset - minimum_offset) & 255;

        m_encoded_data.push_back(static_cast<agbpack_u8>(b0));
        m_encoded_data.push_back(static_cast<agbpack_u8>(b1));
    }

private:
    void write_tag(bool is_reference)
    {
        m_tag_bitmask >>= 1;
        if (!m_tag_bitmask)
        {
            // Tag byte is full.
            // Allocate space for a new one in the output and remember its position.
            m_tag_bitmask = 0x80;
            m_encoded_data.push_back(0);
            m_tag_byte_position = m_encoded_data.size() - 1;
        }

        if (is_reference)
        {
            m_encoded_data[m_tag_byte_position] |= m_tag_bitmask;
        }
    }

    agbpack_u8 m_tag_bitmask = 0;
    size_t m_tag_byte_position = 0;
    std::vector<agbpack_u8>& m_encoded_data;
};

export class lzss_encoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void encode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type(input);

        const auto uncompressed_data = read_input(input, eof);

        const auto encoded_data = encode_internal(uncompressed_data);
        const auto header = header::create(lzss_options::reserved, uncompressed_data.size());

        // Copy header and encoded data to output
        unbounded_byte_writer<OutputIterator> writer(output);
        write32(writer, header.to_uint32_t());
        write(writer, encoded_data.begin(), encoded_data.end());
        write_padding_bytes(writer);
    }

    void vram_safe(bool enable)
    {
        m_vram_safe = enable;
    }

    bool vram_safe() const
    {
        return m_vram_safe;
    }

private:
    template <std::input_iterator InputIterator>
    static std::vector<agbpack_u8> read_input(InputIterator input, InputIterator eof)
    {
        std::vector<agbpack_u8> data;
        std::copy(input, eof, back_inserter(data));
        return data;
    }

    std::vector<agbpack_u8> encode_internal(const std::vector<agbpack_u8>& input)
    {
        // TODO: implement
        //       * Do we implement lookahead optimization?: Yes
        //         * A good test data file is the CC0 license
        std::vector<agbpack_u8> encoded_data;
        match_finder match_finder(input, get_minimum_offset(m_vram_safe) - 1); // TODO: unhardcode. What's somewhat ugly: match_finder uses zero based offfset, whereas global constant uses one based offset
        lzss_bitstream_writer writer(encoded_data);

        size_t current_position = 0;
        while (current_position < input.size())
        {
            auto match = match_finder.find_match(current_position);

            if (match.length() >= minimum_match_length)
            {
                writer.write_reference(match.length(), match.offset());
                current_position += match.length();
            }
            else
            {
                writer.write_literal(input[current_position]);
                current_position += 1;
            }
        }

        return encoded_data;
    }

private:
    bool m_vram_safe = false;
};

}

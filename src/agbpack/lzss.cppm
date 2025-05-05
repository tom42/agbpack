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
#include <ranges>
#include <utility>
#include <vector>
#include "clownlzss.h"

export module agbpack:lzss;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

// Note: In VS 2022, MSVC for x86 bugs if we fully qualify std::size_t in the lzss_sliding_window class template.
// We work around this by importing it (referring to C's global size_t would probably work too).
using size_t = std::size_t;
using std::vector;

inline constexpr size_t minimum_offset = 1;
inline constexpr size_t maximum_offset = 4096;
inline constexpr size_t minimum_vram_safe_offset = 2;
inline constexpr size_t minimum_match_length = 3;
inline constexpr size_t maximum_match_length = 18;
inline constexpr auto literal_cost = 1 + 8;
inline constexpr auto match_cost = 1 + 16;
inline constexpr auto filler_value = -1;
inline constexpr auto maximum_match_distance = 0x1000;
inline constexpr auto bytes_per_value = 1;

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

template <typename LzssReceiver>
concept lzss_receiver = requires(LzssReceiver receiver, agbpack_u8 byte, size_t size)
{
    { receiver.tags(byte) };
    { receiver.literal(byte) };
    { receiver.reference(size, size) };
};

// General case LZSS decoder output receiver.
// Works with any kind of output iterator, including those that cannot be read from.
// In order to decode references it maintains an internal sliding window.
template <typename OutputIterator>
class lzss_decoder_output_receiver final
{
public:
    explicit lzss_decoder_output_receiver(OutputIterator output)
        : m_writer(output) {}

    void tags(agbpack_u8) {}

    void literal(agbpack_u8 literal)
    {
        write8(literal);
    }

    void reference(size_t length, size_t offset)
    {
        while (length--)
        {
            auto byte = m_window.read8(offset);
            write8(byte);
        }
    }

private:
    void write8(agbpack_u8 byte)
    {
        m_writer.write8(byte);
        m_window.write8(byte);
    }

    unbounded_byte_writer<OutputIterator> m_writer;
    lzss_sliding_window<maximum_offset> m_window;
};

// Specialized LZSS decoder output receiver for random access iterators.
// Does not need memory for a separate sliding window because references can be read from the output buffer.
template <std::random_access_iterator RandomAccessIterator>
class lzss_decoder_output_receiver<RandomAccessIterator> final
{
public:
    explicit lzss_decoder_output_receiver(RandomAccessIterator output)
        : m_output(output)
    {}

    void tags(agbpack_u8) {}

    void literal(agbpack_u8 literal)
    {
        write8(literal);
    }

    void reference(size_t length, size_t offset)
    {
        while (length--)
        {
            auto byte = *(m_output - make_signed(offset));
            write8(byte);
        }
    }

private:
    void write8(agbpack_u8 byte)
    {
        *m_output++ = byte;
    }

    RandomAccessIterator m_output;
};

export class lzss_decoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type<InputIterator>(); // TODO: probably we want to either remove this or extend it with the output iterator?

        byte_reader<InputIterator> reader(input, eof);
        lzss_decoder_output_receiver<OutputIterator> receiver(output);

        decode_internal(reader, receiver);
    }

    template <std::input_iterator InputIterator, lzss_receiver LzssReceiver>
    void decode(InputIterator input, InputIterator eof, LzssReceiver receiver)
    {
        static_assert_input_type<InputIterator>();
        byte_reader<InputIterator> reader(input, eof);
        decode_internal(reader, receiver);
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
    template <std::input_iterator InputIterator, typename LzssReceiver>
    void decode_internal(byte_reader<InputIterator>& reader, LzssReceiver& receiver)
    {
        static_assert_input_type<InputIterator>();

        auto header = header::parse_for_type(compression_type::lzss, read32(reader));
        if (!header)
        {
            throw decode_exception();
        }

        unsigned int tag_mask = 0;
        agbpack_u8 tags = 0;
        size_t nbytes_written = 0;

        while (nbytes_written < header->uncompressed_size())
        {
            tag_mask >>= 1;
            if (!tag_mask)
            {
                tags = read8(reader);
                tag_mask = 0x80;
                receiver.tags(tags);
            }

            if (tags & tag_mask)
            {
                auto b0 = read8(reader);
                auto b1 = read8(reader);
                size_t length = ((b0 >> 4) & 0xf) + minimum_match_length;
                size_t offset = (((b0 & 0xfu) << 8) | b1) + minimum_offset;

                assert(in_closed_range(length, minimum_match_length, maximum_match_length) && "lzss_decoder is broken");
                assert(in_closed_range(offset, minimum_offset, maximum_offset) && "lzss_decoder is broken");

                throw_if_bad_reference(length, offset, nbytes_written, header->uncompressed_size());

                receiver.reference(length, offset);
                nbytes_written += length;
            }
            else
            {
                receiver.literal(read8(reader));
                ++nbytes_written;
            }
        }

        parse_padding_bytes(reader);
    }

    void throw_if_bad_reference(size_t length, size_t offset, size_t nbytes_written, size_t uncompressed_size)
    {
        throw_if_not_vram_safe(offset);
        throw_if_outside_sliding_window(offset, nbytes_written);
        throw_if_reference_overflows_uncompressed_size(length, nbytes_written, uncompressed_size);
    }

    void throw_if_not_vram_safe(size_t offset)
    {
        if (offset < get_minimum_offset(m_vram_safe))
        {
            throw decode_exception("encoded data is not VRAM safe");
        }
    }

    static void throw_if_outside_sliding_window(size_t offset, size_t nbytes_written)
    {
        if (offset > nbytes_written)
        {
            throw decode_exception("reference outside of sliding window");
        }
    }

    static void throw_if_reference_overflows_uncompressed_size(size_t length, size_t nbytes_written, size_t uncompressed_size)
    {
        if ((nbytes_written + length) > uncompressed_size)
        {
            throw decode_exception();
        }
    }

    bool m_vram_safe = false;
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
class match final
{
public:
    explicit match(size_t length, size_t offset)
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
class greedy_match_finder final
{
public:
    // Note: greedy_match_finder does not own input
    explicit greedy_match_finder(const vector<agbpack_u8>& input, size_t minimum_match_offset)
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

        // Outer loop: step through sliding window, starting at longest possible offset.
        size_t offset = std::min(current_position, maximum_offset);
        for (; offset > m_minimum_match_offset; --offset)
        {
            // Inner loop: check for match at current position, increasing match length each iteration.
            // Note: close to the end of input, iterating over the entire range [0, maximum_match_length]
            // is pointless if that exceeds the available input. This could easily be optimized, but
            // considering we're doing linear search that would be a micro optimization.
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
    const vector<agbpack_u8>& m_input;
    size_t m_minimum_match_offset;
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
class lzss_bitstream_writer final
{
public:
    // Note: lzss_bitstream_writer does not own encoded_data
    explicit lzss_bitstream_writer(vector<agbpack_u8>& encoded_data)
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
    vector<agbpack_u8>& m_encoded_data;
};

export class lzss_encoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void encode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type<InputIterator>();

        const auto uncompressed_data = vector<agbpack_u8>(input, eof);
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
    vector<agbpack_u8> encode_internal(const vector<agbpack_u8>& input)
    {
        vector<agbpack_u8> encoded_data;
        greedy_match_finder match_finder(input, get_minimum_offset(m_vram_safe) - 1); // TODO: unhardcode. What's somewhat ugly: greedy_match_finder uses zero based offfset, whereas global constant uses one based offset
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

export class optimal_lzss_encoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void encode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type<InputIterator>();

        const auto uncompressed_data = vector<agbpack_u8>(input, eof);
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
    vector<agbpack_u8> encode_internal(const vector<agbpack_u8>& uncompressed_data)
    {
        const auto [matches, total_matches] = find_optimal_matches(uncompressed_data);
        return encode_matches(uncompressed_data, matches, total_matches);
    }

    std::pair<ClownLZSS::Matches, size_t> find_optimal_matches(const vector<agbpack_u8>& uncompressed_data)
    {
        ClownLZSS::Matches matches;
        size_t total_matches;
        auto match_cost_callback = vram_safe() ? get_match_cost_vram_safe : get_match_cost;

        if (!ClownLZSS::FindOptimalMatches(
            filler_value,
            maximum_match_length,
            maximum_match_distance,
            nullptr,
            literal_cost,
            match_cost_callback,
            uncompressed_data.data(),
            bytes_per_value,
            uncompressed_data.size() / bytes_per_value,
            &matches,
            &total_matches,
            nullptr))
        {
            throw encode_exception("optimal LZSS encoding failed. That should not happen, unless the system is extremely low on memory");
        }

        return std::make_pair(std::move(matches), total_matches);
    }

    vector<agbpack_u8> encode_matches(const vector<agbpack_u8>& uncompressed_data, const ClownLZSS::Matches& matches, size_t total_matches)
    {
        vector<agbpack_u8> encoded_data;
        lzss_bitstream_writer writer(encoded_data);

        for (const auto& match : std::ranges::subrange(&matches[0], &matches[total_matches]))
        {
            if (CLOWNLZSS_MATCH_IS_LITERAL(&match))
            {
                writer.write_literal(uncompressed_data[match.destination]);
            }
            else
            {
                writer.write_reference(match.length, match.destination - match.source);
            }
        }

        return encoded_data;
    }

    static size_t get_match_cost(const size_t, const size_t length, void* const)
    {
        if (length < minimum_match_length)
        {
            return 0;
        }

        return match_cost;
    }

    static size_t get_match_cost_vram_safe(const size_t distance, const size_t length, void* const)
    {
        if ((length < minimum_match_length) || (distance < minimum_vram_safe_offset))
        {
            return 0;
        }

        return match_cost;
    }

    bool m_vram_safe = false;
};

}

// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <iterator>
#include <utility>
#include <vector>

export module agbpack:lzss;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

// TODO: reconsider the use of size_t here: on a 16 bit platform this is too small
inline constexpr unsigned int sliding_window_size = 4096; // TODO: rename this to maximum_offset or something?
inline constexpr unsigned int minimum_match_length = 3;
inline constexpr unsigned int maximum_match_length = 18;

// Sliding window for LZSS decoder. Used when the output iterator does not allow random access.
// * Maintains an internal write position which wraps around when the window is written to.
// * Allows reading relative to the write position. The final read position wraps around.
//   Note: the sliding window is not initialized. Reading from a position that has not yet
//   been written to returns uninitialized data.
template <std::size_t Size>
class lzss_sliding_window final
{
public:
    agbpack_u8 read8(std::size_t offset)
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
    static constexpr std::size_t index_mask = Size - 1;

    void assert_read_allowed([[maybe_unused]] std::size_t offset)
    {
        // Required for g++ 14.2.0, which ignores [[maybe_unused]] here
        std::ignore = offset;

        assert((m_nbytes_written > 0) && "Cannot read from empty sliding window");
        assert((offset > 0) && "Cannot read from current write position");
        assert((offset <= Size) && "Offset is too big");
        assert((offset <= m_nbytes_written) && "Offset is too big for amount of data currently in sliding window");
    }

    std::size_t m_nbytes_written = 0;
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

    void copy_from_output(unsigned int nbytes, std::size_t offset)
    {
        // TODO: must check if this under/overflows!
        //       Note: this CAN happen at runtime when the encoded stream is corrupt, so cannot be just an assert()
        // TODO: well if we do this here we need to do so in the specialization for random_access_iterator too, so it's better done in the actual decoder, no?
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

private:
    byte_writer<OutputIterator> m_writer;
    lzss_sliding_window<sliding_window_size> m_window;
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

    void copy_from_output(unsigned int nbytes, std::size_t offset)
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
        // TODO: Do we want to have a mode where the decoder is explicitly asked to decode VRAM safe data?
        //       Such a thing would be used as verification. Such a decoder would then bark if the data
        //       is not actually VRAM safe.
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
                std::size_t offset = (((b0 & 0xfu) << 8) | b1) + 1;

                assert((minimum_match_length <= nbytes) && (nbytes <= maximum_match_length) && "lzss_decoder is broken");
                assert((1 <= offset) && (offset <= sliding_window_size) && "lzss_decoder is broken");

                // TODO: tests for invalid input
                //       * read outside of sliding window
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
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
class match final
{
public:
    match(std::size_t length, std::size_t offset)
        : m_length(length)
        , m_offset(offset)
    {}

    std::size_t length() const { return m_length; }

    std::size_t offset() const { return m_offset; }

    bool operator==(const match&) const = default;

private:
    std::size_t m_length;
    std::size_t m_offset;
};

// TODO: document or otherwise make clear that this does NOT own the vector?
AGBPACK_EXPORT_FOR_UNIT_TESTING
class match_finder final
{
public:
    explicit match_finder(const std::vector<agbpack_u8>& input) : m_input(input) {}

    match find_match(std::size_t current_position)
    {
        match best_match(0, 0);

        // TODO: implement this.
        //       Basic idea: two nested loops.
        //       * Outer loop: search backwards through sliding window (towards larger offset, away from current position => Nope, wrong, we start at longest offset and keep doing so while it is not 0)
        //         * Inner loop: search forwards, increasing string length.
        //           * Two problems here:
        //             * What's the initial length to search for? minimum_match_length, no?
        //               * Nope, since we must compare all characters inbetween
        //             * The maximum match length to search for is of course maximum_match_length, but towards the end of the buffer it may be shorter

        // TODO: obviously this is crap: we need to clamp offset at the beginning of the buffer
        // TODO: and later we also need to take into account VRAM safety, but we can worry about this later, I think
        std::size_t offset = sliding_window_size;
        (void)offset; // TODO: remove

        for (std::size_t length = 0; length < maximum_match_length; ++length)
        {
            if (current_position + length >= m_input.size())
            {
                // TODO: test condition/branch?
                // TODO: document what this does? (check whether end of input/lookahead buffer is reached)
                break;
            }
        }

        return best_match;
    }

private:
    const std::vector<agbpack_u8>& m_input;
};

// TODO: document or otherwise make clear that this does NOT own the vector?
class lzss_bitstream_writer final
{
public:
    explicit lzss_bitstream_writer(std::vector<agbpack_u8>& encoded_data)
        : m_encoded_data(encoded_data)
    {}

    void write_literal(agbpack_u8 literal)
    {
        if (m_count % 8 == 0)
        {
            m_encoded_data.push_back(0);
        }
        ++m_count;

        m_encoded_data.push_back(literal);
    }

private:
    int m_count = 0;
    std::vector<agbpack_u8>& m_encoded_data;
};

// TODO: implement
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
private:
    template <std::input_iterator InputIterator>
    static std::vector<agbpack_u8> read_input(InputIterator input, InputIterator eof)
    {
        std::vector<agbpack_u8> data;
        std::copy(input, eof, back_inserter(data));
        return data;
    }

    static std::vector<agbpack_u8> encode_internal(const std::vector<agbpack_u8>& input)
    {
        // TODO: implement
        //       * Do it simple first, using linear search
        //         * Note: "Matches overlapping lookahead on LZ77/LZSS with suffix trees"
        //           * https://stackoverflow.com/questions/31347593/matches-overlapping-lookahead-on-lz77-lzss-with-suffix-trees
        //           * Well we won't worry about suffix trees, but definitely here is the example we were looking for:
        //             * We were looking for an example where the sliding window and the lookahead buffer overlap
        //             * This is the original text that made me aware of that problem/feature
        //               * What does GBACrusher do?
        //               * What does CUE lzss do?
        //               * If we implement it like this, can the GBA BIOS decode it?
        //       * Do we implement lookahead optimization?
        //       * Later implement optimal parse
        //       * NEXT:
        //         * Implement references
        std::vector<agbpack_u8> encoded_data;
        match_finder match_finder(input);
        lzss_bitstream_writer writer(encoded_data);

        // TODO: this is a bogus implementation that passes our tests:
        //       * We simply copy input to output
        //       * Every 8 bytes we write a fake tag byte
        std::size_t current_position = 0;
        while (current_position < input.size())
        {
            auto match = match_finder.find_match(current_position);

            if (match.length() >= minimum_match_length)
            {
                throw "TODO: yikes: this branch is not yet implemented";
            }
            else
            {
                writer.write_literal(input[current_position++]);
            }
        }

        return encoded_data;
    }
};

}

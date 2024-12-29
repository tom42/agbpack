// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <iterator>
#include <utility>

export module agbpack:lzss;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

// TODO: reconsider the use of size_t here: on a 16 bit platform this is too small
inline constexpr unsigned int sliding_window_size = 4096;
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

}

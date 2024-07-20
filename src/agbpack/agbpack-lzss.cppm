// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <array>
#include <bit>
#include <cassert>
#include <iterator>

export module agbpack:lzss;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

constexpr std::size_t sliding_window_size = 4096;
constexpr std::size_t minimum_match_length = 3;
constexpr std::size_t maximum_match_length = 18;

// Sort of ringbuffer:
// * Maintains an internal write position which wraps around when the buffer is written to.
// * Allows reading from arbitrary position, where the read index wraps around.
//   Note: the buffer is not initialized. Reading from a position that has not yet been
//   written to returns uninitialized data.
template <std::size_t Size>
class ringbuffer final // TODO: maybe rename it to sliding window again and special case it for that purpose. If we pass in a displacement into read8 we then don't need any other support. like size()
{
public:
    // TODO: size is not quite what we're after. We simply want to know the number of bytes written so far
    //       => Is this not simply not our problem? => Can this not the decoder know? => Well he can ask the writer, he knows
    std::size_t size()
    {
        return m_write_position;
    }

    agbpack_u8 read8(std::size_t position)
    {
        // TODO: here we want to wrap around, no?
        // TODO: have some debug mode where we assert that no uninitialized position is read from?
        return m_buf[position & position_mask];
    }

    void write8(agbpack_u8 byte)
    {
        m_buf[m_write_position] = byte;
        m_write_position = (m_write_position + 1) & position_mask;
    }

private:
    static_assert(std::popcount(Size) == 1, "Size must be a power of 2 for index calculations using operator & to work");
    static constexpr std::size_t position_mask = Size - 1;
    std::size_t m_write_position = 0;
    std::array<agbpack_u8, Size> m_buf;
};

// TODO: specialize this for the case when the output iterator is a random access iterator?
//       * Well yes but if we do this we must run all of our tests twice. Not that that's much of a problem, though.
template <std::output_iterator<agbpack_io_datatype> OutputIterator>
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

    void copy_from_output(unsigned int nbytes, std::size_t displacement)
    {
        // TODO: must check if this under/overflows! (well since all is unsigned, can't we just do the comparison unsigned? no need to have ssize_t)
        //       * The important bit here is this: this CAN happen at runtime when the encoded stream is corrupt, so cannot be just an assert()
        std::size_t src = m_window.size() - displacement - 1;
        while (nbytes--)
        {
            auto byte = m_window.read8(src++);
            write8(byte);
        }
    }

    bool done() const
    {
        return m_writer.done();
    }

private:
    byte_writer<OutputIterator> m_writer;
    ringbuffer<sliding_window_size> m_window;
};

export class lzss_decoder final
{
public:
    template <std::input_iterator InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        // TODO: Do we want to have a mode where the decoder is explicitly asked to decode VRAM safe data?
        //       Such a thing would be used as verification. Such a decoder would then bark if the data
        //       is not actually VRAM safe.
        static_assert_input_type(input);

        byte_reader<InputIterator> reader(input, eof);
        auto header = header::parse_for_type(compression_type::lzss, reader.read32());
        if (!header)
        {
            throw bad_encoded_data();
        }

        lzss_byte_writer<OutputIterator> writer(header->uncompressed_size(), output);

        unsigned int mask = 0;
        unsigned int flags = 0;

        while (!writer.done())
        {
            mask >>= 1;
            if (!mask)
            {
                flags = reader.read8(); // TODO: test if we hit EOF here (reading flag byte)
                mask = 0x80;
            }

            if (flags & mask)
            {
                // TODO: test if we hit EOF whily reading a back reference
                auto b0 = reader.read8();
                auto b1 = reader.read8();
                unsigned int nbytes = ((b0 >> 4) & 0xf) + minimum_match_length;
                std::size_t displacement = ((b0 & 0xfu) << 8) | b1;

                assert((minimum_match_length <= nbytes) && (nbytes <= maximum_match_length) && "lzss_decoder is broken");
                assert((displacement < sliding_window_size) && "lzss_decoder is broken");

                // TODO: tests for invalid input
                //       * too many bytes written
                //       * read outside of sliding window
                writer.copy_from_output(nbytes, displacement);
            }
            else
            {
                // TODO: test: EOF input when reading single literal byte
                // TODO: test: too much ouput when writing single literal byte
                auto byte = reader.read8();
                writer.write8(byte);
            }
        }


        // TODO: parse padding bytes
    }
};

}

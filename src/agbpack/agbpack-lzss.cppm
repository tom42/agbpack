// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>
#include <vector> // TODO: remove if not needed anymore

export module agbpack:lzss;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

// TODO: maybe rename to sliding_window_writer?
// TODO: specialize this for the case when the output iterator is a random access iterator?
//       * Well yes but if we do this we must run all of our tests twice. Not that that's much of a problem, though.
template <std::output_iterator<agbpack_io_datatype> OutputIterator>
class sliding_window final
{
public:
    sliding_window(agbpack_u32 uncompressed_size, OutputIterator output)
        : m_writer(uncompressed_size, output) {}

    void write8(agbpack_u8 byte)
    {
        // TODO: would we want to limit the size of the sliding window? => Well yes but probably we should then use a deque?
        m_writer.write8(byte);
        m_window.push_back(byte);
    }

    void copy_from_window(int nbytes, std::size_t displacement)
    {
        std::size_t src = m_window.size() - displacement - 1; // TODO: must check if this under/overflows!
        while (nbytes--)
        {
            auto byte = m_window[src++];
            write8(byte);
        }
    }

    bool done() const
    {
        return m_writer.done();
    }

private:
    byte_writer<OutputIterator> m_writer;
    std::vector<agbpack_u8> m_window;
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

        sliding_window<OutputIterator> sliding_window(header->uncompressed_size(), output);

        unsigned int mask = 0;
        unsigned int flags = 0;

        while (!sliding_window.done())
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
                int nbytes = ((b0 >> 4) & 0xf) + 3;
                std::size_t displacement = ((b0 & 0xfu) << 8) | b1; // TODO: assert this is in the range 0..??? (2047?) => Well yes, but maybe do that inside the sliding window?

                // TODO: tests for invalid input
                //       * too many bytes written
                //       * read outside of sliding window
                sliding_window.copy_from_window(nbytes, displacement);
            }
            else
            {
                // TODO: test: EOF input when reading single literal byte
                // TODO: test: too much ouput when writing single literal byte
                auto byte = reader.read8();
                sliding_window.write8(byte);
            }
        }


        // TODO: parse padding bytes
    }
};

}

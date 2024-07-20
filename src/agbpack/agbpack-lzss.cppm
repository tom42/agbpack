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

template <std::output_iterator<agbpack_io_datatype> OutputIterator>
class sliding_window final
{
public:
    sliding_window(OutputIterator /*output*/) {}
private:
    OutputIterator m_output;
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

        byte_writer<OutputIterator> writer(header->uncompressed_size(), output);

        // TODO: that's a a rather temporary hack. Also, shouldn't we use a deque?
        //       => Well what we should do is we should wrap this into a class so we can easily replace it
        std::vector<agbpack_u8> sliding_window;

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
                // TODO: read back reference correctly
                // TODO: test if we hit EOF whily reading a back reference
                auto b0 = reader.read8();
                auto b1 = reader.read8();
                int nbytes = ((b0 >> 4) & 0xf) + 3;
                std::size_t displacement = ((b0 & 0xfu) << 8) | b1;
                std::size_t src = sliding_window.size() - displacement - 1; // TODO: must check if this under/overflows!

                while (nbytes--)
                {
                    // TODO: this requires us to have an RANDOM ITERATOR!
                    // TODO: well we could also write a version that has its own sliding window, no?
                    //       => Well since our test setup actually prefers that, let's do just that.
                    // TODO: tests for invalid input
                    //       * too many bytes written
                    //       * read outside of sliding window
                    // TODO: now we need to copy stuff from our sliding window (copy from dest - disp - 1)

                    auto byte = sliding_window[src++];
                    writer.write8(byte);
                    sliding_window.push_back(byte);
                }
            }
            else
            {
                // TODO: test: EOF input when reading single literal byte
                // TODO: test: too much ouput when writing single literal byte
                auto byte = reader.read8();
                writer.write8(byte);
                sliding_window.push_back(byte);
            }
        }


        // TODO: parse padding bytes
    }
};

}

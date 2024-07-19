// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>

export module agbpack:lzss;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

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

        // TODO: decode stuff
        byte_writer<OutputIterator> writer(header->uncompressed_size(), output);



        unsigned int mask = 0;
        unsigned int flags = 0;
        while (!writer.done())
        {
            mask >>= 1;
            if (!mask)
            {
                flags = reader.read8(); // TODO: test if we hit EOF here
                mask = 0x80;
            }

            // TODO: test: EOF input when reading single literal byte
            // TODO: test: too much ouput when writing single literal byte
            writer.write8(reader.read8());
        }


        // TODO: parse padding bytes
    }
};

}

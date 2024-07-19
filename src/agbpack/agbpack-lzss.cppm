// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>

export module agbpack:lzss;
import :common;
import :header;

namespace agbpack
{

export class lzss_decoder final
{
public:
    template <std::input_iterator InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator /*output*/)
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

        // TODO: parse/validate header
        // TODO: decode stuff
        // TODO: parse padding bytes
    }
};

}

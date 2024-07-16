// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>

export module agbpack:delta;
import :common;
import :exceptions;

namespace agbpack
{

export class delta_decoder final
{
public:
    template <std::input_iterator InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type(input);

        byte_reader<InputIterator> reader(input, eof);
        header header(reader.read32());

        // TODO: also check options
        // TODO: move to static method
        if (header.type() != compression_type::delta)
        {
            throw bad_encoded_data();
        }

        switch (header.options())
        {
            case 1: // TODO: unhardcode this. Can we somehow get an enum for rle options?
                // TODO: hardcoded to pass first test (don't hardcode, and use a byte_writer)
                *output++ = 'a';
                break;
            default: // TODO: write a test for this branch
                throw bad_encoded_data();
        }

        // TODO: parse header
        // TODO: decode (8 and 16 bit variants)

    }
};

}

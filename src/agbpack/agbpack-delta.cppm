// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>

export module agbpack:delta;
import :common;

namespace agbpack
{

// TODO: this should go into its own partition
//       => But so should rle_decoder
//       => And the common stuff at the top should go into another one
export class delta_decoder final
{
public:
    // TODO: 'unsigned char': use agbpack_io_datatype again. Problem: I don't have access to that yet.
    template <std::input_iterator InputIterator, std::output_iterator<unsigned char> OutputIterator>
    void decode(InputIterator input, InputIterator /*eof*/, OutputIterator output)
    {
        static_assert_input_type(input);
        // TODO: parse header
        // TODO: decode (8 and 16 bit variants)

        // TODO: hardcoded to pass first test (don't hardcode, and use a byte_writer)
        *output++ = 'a';
    }
};

}

// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>

export module agbpack:huffman;
import :common;

namespace agbpack
{

export class huffman_decoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator /*output*/)
    {
        static_assert_input_type(input);

        byte_reader<InputIterator> reader(input, eof);
        reader.read32();
        // TODO: parse header: compression typename
        // TODO: at some point we'll have to parse compression options, but which do we support?
        //       => Depends mostly on what the GBA BIOS supports, no? If it can do 1 and 2 bit we'll do so too, no?

        // TODO: parse padding bytes here
    }
private:
};

}

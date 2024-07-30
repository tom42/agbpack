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
    void decode(InputIterator input, InputIterator /*eof*/, OutputIterator /*output*/)
    {
        static_assert_input_type(input);
        // TODO: parse padding bytes here
    }
private:
};

}

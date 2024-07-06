// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>

export module agbpack;

namespace agbpack
{

export class rle_decoder final
{
public:
    // TODO: do we want to check what output points to? Or do we simply check that iterators point all to the same element type?
    //       do we restrict ourselves to byte/unsigned char/signed char/char?
    template <typename OutputIterator>
    void decode(OutputIterator output)
    {
        // TODO: actually decode stuff from input stream
        *output++ = 'a';
        *output++ = 'b';
        *output++ = 'c';
    }
private:
};

}

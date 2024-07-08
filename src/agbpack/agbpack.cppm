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
    // TODO: well we can't even assume that InputIterator has the same element type as output iterator:
    //       ifstream::rdbuf() returns something that works with char, but if we'd like to decompress to e.g. vector<unsigned char>, then there you go. ouch.
    template <typename InputIterator, typename OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        // TODO: actually decode stuff from input stream
        //       currently we assume a particular file with three literals at the end only, which we copy to output, after skipping the header and the one and only flag byte)
        (void)eof; // TODO: remove: should not ignore eof, but instead check it before reading more data (if according to stream parsing we need more data but we reached eof, the stream is corrupt)
        ++input;
        ++input;
        ++input;
        ++input;
        ++input;
        // TODO: figure out what to do about the casts here:
        //       * I am not sure whether we can ensure that input and output works on the same element type
        //         That very much depends on what we can do with input stream stuff.
        //       * Eithwe way, we can maybe abstract things a bit here
        *output++ = static_cast<unsigned char>(*input++);
        *output++ = static_cast<unsigned char>(*input++);
        *output++ = static_cast<unsigned char>(*input++);
    }
private:
};

}

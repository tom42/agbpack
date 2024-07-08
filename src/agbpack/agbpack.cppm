// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>
#include <type_traits>

export module agbpack;

namespace agbpack
{

export class rle_decoder final
{
public:
    // TODO: do we want to check what output points to? Or do we simply check that iterators point all to the same element type?
    //       do we restrict ourselves to byte/unsigned char/signed char/char?
    // TODO: For the time being we process unsigned char only. Will figure out later whether we need anything else.
    //       We might at the very least want to have a typedef/using alias for this, though.
    //       Not sure whether we want to allow stuff such as input reads char, output takes unsigned char.
    template <std::input_iterator InputIterator, std::output_iterator<unsigned char> OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert(
            std::is_same_v<std::remove_cv_t<std::remove_reference_t<decltype(*input)>>,
            unsigned char>,
            "Input iterator should read values of type unsigned char");

        // TODO: actually decode stuff from input stream
        //       currently we assume a particular file with three literals at the end only, which we copy to output, after skipping the header and the one and only flag byte)
        (void)eof; // TODO: remove: should not ignore eof, but instead check it before reading more data (if according to stream parsing we need more data but we reached eof, the stream is corrupt)
        ++input;
        ++input;
        ++input;
        ++input;
        ++input;
        *output++ = *input++;
        *output++ = *input++;
        *output++ = *input; // TODO: hack: do not increment input last time. Doing so triggers exceptions.
    }
private:
};

}

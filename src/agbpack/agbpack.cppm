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

        (void)eof; // TODO: remove: should not ignore eof, but instead check it before reading more data (if according to stream parsing we need more data but we reached eof, the stream is corrupt)

        // TODO: hack: "process header"
        // TODO: in principle, each read operation should check whether input != eof, no? (Also later during decompression)
        ++input;
        unsigned int uncompressed_size = *input++;
        uncompressed_size += static_cast<unsigned int>((*input++) << 8u); // TODO: can we get rid of these casts?
        uncompressed_size += static_cast<unsigned int>((*input++) << 8u);
        (void)uncompressed_size;

        // TODO: actually decode stuff from input stream
        //       currently we assume a particular file with three literals at the end only, which we copy to output, after skipping the header and the one and only flag byte)
        ++input;
        while (uncompressed_size--)
        {
            *output++ = *input;
            // TODO: hack: in some cases, incrementing after last read causes exceptions in streams.
            //             Reason: we read past EOF.
            //             As for why it only happens sometimes: compressed data is padded to next multiple of 4 bytes.
            //             If there are padding bytes, then no exception happens.
            if (uncompressed_size >= 1)
            {
                input++;
            }
        }
    }
private:
};

}

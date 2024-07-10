// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cstdint>
#include <iterator>
#include <type_traits>

export module agbpack;

namespace agbpack
{

// TODO: do we really want this? Shouldn't we simply use uint8_t, uint16_t etc.?
using agbpack_byte = uint_fast8_t;

// TODO: flesh this out
// TODO: do we want to ensure *input points to something we understand?
template <std::input_iterator InputIterator>
class byte_reader final
{
public:
    byte_reader(InputIterator input, InputIterator eof)
        : m_input(input)
        , m_eof(eof)
    {}

    // TODO: not sure this is such a good idea: shouldn't we simply use directly something usable? or fixed size types?
    agbpack_byte read8()
    {
        // TODO: this is where we'd throw if we read past the end of the input:
        //       If according to parsing we need more input but reached the end of the stream, then the stream is corrupt.
        return *m_input++;
    }

private:
    InputIterator m_input;
    InputIterator m_eof;
};

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

        // TODO: need to specify this because of some odd warning (-Wctad-maybe-unsupported). Do we want this?
        byte_reader<InputIterator> reader(input, eof);

        // TODO: hack: "process header"
        // TODO: in principle, each read operation should check whether input != eof, no? (Also later during decompression)
        reader.read8(); // TODO: skip type byte: should verify this!
        // TODO: read uncompressed size. Do we verify this in any way? It should be a multiple of 4, but probably we don't enforce this. This is just a GBA requirement, really.
        unsigned int uncompressed_size = reader.read8();
        uncompressed_size += reader.read8() * 256;
        uncompressed_size += reader.read8() * 256 * 256;


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

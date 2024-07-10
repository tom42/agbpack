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
using agbpack_u8 = uint_fast8_t;
using agbpack_u32 = uint_fast32_t;

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
    agbpack_u8 read8()
    {
        // TODO: this is where we'd throw if we read past the end of the input:
        //       If according to parsing we need more input but reached the end of the stream, then the stream is corrupt.
        return *m_input++;
    }

    agbpack_u32 read24()
    {
        agbpack_u32 result = read8();
        result += read8() * 256;
        result += read8() * 256 * 256;
        return result;
    }

private:
    InputIterator m_input;
    InputIterator m_eof;
};

// TODO: flesh this out
// TODO: do we want to ensure T is something we understand?
// TODO: wouldn't we want to use std::output_iterator again?
template <typename OutputIterator>
class byte_writer final
{
public:
    byte_writer(OutputIterator output) : m_output(output) {}

    void write8(agbpack_u8 byte)
    {
        *m_output++ = byte;
    }

private:
    OutputIterator m_output;
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

        byte_reader<InputIterator> reader(input, eof);
        byte_writer<OutputIterator> writer(output);

        // TODO: hack: "process header"
        // TODO: in principle, each read operation should check whether input != eof, no? (Also later during decompression)
        reader.read8(); // TODO: skip type byte: should verify this!

        // TODO: do we use size_t here or not?
        std::size_t uncompressed_size = reader.read24();
        std::size_t decompressed = 0;

        // TODO: in some cases, incrementing after last read causes exceptions in streams.
        //       Reason: we read past EOF.
        //       As for why it only happens sometimes: compressed data is padded to next multiple of 4 bytes.
        //       If there are padding bytes, then no exception happens. => Well test with a vector, no?

        while (decompressed < uncompressed_size)
        {
            auto flag = reader.read8();
            if (flag & 0x80)
            {
                // TODO: detect when we go past uncompressed_size
                auto n = (flag & 127) + 3;
                auto byte = reader.read8();
                decompressed += n;
                while (n--)
                {
                    writer.write8(byte);
                }
            }
            else
            {
                // TODO: detect when we go past uncompressed_size
                auto n = (flag & 127) + 1;
                decompressed += n;
                while (n--)
                {
                    writer.write8(reader.read8());
                }
            }
        }
    }
};

}

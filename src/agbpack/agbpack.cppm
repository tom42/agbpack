// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cstdint>
#include <iterator>
#include <type_traits>

export module agbpack;
export import agbpack.exceptions;

namespace agbpack
{

using agbpack_u8 = uint_fast8_t;
using agbpack_u32 = uint_fast32_t;

// TODO: do we want to ensure *input points to something we understand?
template <std::input_iterator InputIterator>
class byte_reader final
{
public:
    byte_reader(InputIterator input, InputIterator eof)
        : m_input(input)
        , m_eof(eof)
    {}

    agbpack_u8 read8()
    {
        if (m_input == m_eof)
        {
            throw bad_compressed_data();
        }
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

// TODO: do we want to ensure T is something we understand?
// TODO: wouldn't we want to use std::output_iterator again?
template <typename OutputIterator>
class byte_writer final
{
public:
    explicit byte_writer(OutputIterator output) : m_output(output) {}

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

        // TODO: hack: verify header. Suggestion: we read the header at once (that is, 4 bytes. Only then do we verify it)
        // Bit 0-3   Reserved => should definitely test this for zero
        // Bit 4-7   Compressed type (must be 3 for run-length) => should definitely test this, too
        reader.read8();

        agbpack_u32 uncompressed_size = reader.read24();
        agbpack_u32 decompressed = 0;

        // TODO: Input should be padded to a multiple of 4 bytes.
        //       Question is then, should we require these padding bytes and skip them?
        //       If we want to be able to decompress multiple files from a single stream, then yes. If not, then not.
        while (decompressed < uncompressed_size)
        {
            auto flag = reader.read8();
            if (flag & 0x80)
            {
                // TODO: detect when we go past uncompressed_size
                agbpack_u32 n = (flag & 127) + 3;
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
                agbpack_u32 n = (flag & 127) + 1;
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

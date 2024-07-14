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

using agbpack_u8 = uint8_t;
using agbpack_u32 = uint32_t;

template <std::input_iterator InputIterator>
class byte_reader final
{
public:
    explicit byte_reader(InputIterator input, InputIterator eof)
        : m_input(input)
        , m_eof(eof)
    {}

    agbpack_u8 read8()
    {
        if (m_input == m_eof)
        {
            throw bad_encoded_data();
        }
        return *m_input++;
    }

    agbpack_u32 read32()
    {
        agbpack_u32 result = read8();
        result += read8() * 256;
        result += read8() * 256 * 256;
        result += read8() * 256 * 256 * 256;
        return result;
    }

private:
    InputIterator m_input;
    InputIterator m_eof;
};

template <typename OutputIterator>
class byte_writer final
{
public:
    explicit byte_writer(agbpack_u32 uncompressed_size, OutputIterator output)
        : m_uncompressed_size(uncompressed_size)
        , m_output(output)
    {}

    void write8(agbpack_u8 byte)
    {
        if (done())
        {
            throw bad_encoded_data();
        }

        ++m_nuncompressed_bytes;
        *m_output++ = byte;
    }

    bool done() const
    {
        return m_nuncompressed_bytes >= m_uncompressed_size;
    }

private:
    agbpack_u32 m_uncompressed_size;
    agbpack_u32 m_nuncompressed_bytes = 0;
    OutputIterator m_output;
};

enum class compression_type
{
    rle = 3
};

class header final
{
public:
    explicit header(uint32_t header_data) : m_header_data(header_data) {}

    compression_type type() const
    {
        return static_cast<compression_type>((m_header_data >> 4) & 0xf);
    }

    uint32_t options() const
    {
        return m_header_data & 0xf;
    }

    uint32_t uncompressed_size() const
    {
        return m_header_data >> 8;
    }

private:
    uint32_t m_header_data;
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
        static_assert_input_type(input);

        byte_reader<InputIterator> reader(input, eof);
        header header(reader.read32());
        verify_header(header);

        // TODO: Input should be padded to a multiple of 4 bytes.
        //       Question is then, should we require these padding bytes and skip them?
        //       If we want to be able to decompress multiple files from a single stream, then yes. If not, then not.
        byte_writer<OutputIterator> writer(header.uncompressed_size(), output);
        while (!writer.done())
        {
            auto flag = reader.read8();
            if (flag & 0x80)
            {
                agbpack_u32 n = (flag & 127) + 3;
                auto byte = reader.read8();
                while (n--)
                {
                    writer.write8(byte);
                }
            }
            else
            {
                agbpack_u32 n = (flag & 127) + 1;
                while (n--)
                {
                    writer.write8(reader.read8());
                }
            }
        }
    }

private:
    static void verify_header(const header& header)
    {
        if (header.type() != compression_type::rle)
        {
            throw bad_encoded_data();
        }
        if (header.options() != 0)
        {
            throw bad_encoded_data();
        }
    }

    template <typename InputIterator>
    static void static_assert_input_type(InputIterator& input)
    {
        static_assert(
            std::is_same_v<std::remove_cv_t<std::remove_reference_t<decltype(*input)>>,
            unsigned char>,
            "Input iterator should read values of type unsigned char");
    }
};

}

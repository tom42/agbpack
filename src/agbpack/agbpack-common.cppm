// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cstdint>
#include <iterator>

export module agbpack:common;
import :exceptions;

namespace agbpack
{

using agbpack_io_datatype = unsigned char;
using agbpack_u8 = uint8_t;
using agbpack_u16 = uint16_t;
using agbpack_u32 = uint32_t;

template <std::input_iterator InputIterator>
class byte_reader final
{
public:
    explicit byte_reader(InputIterator input, InputIterator eof)
        : m_input(input)
        , m_eof(eof)
    {}

    agbpack_u32 nbytes_read() const
    {
        return m_nbytes_read;
    }

    agbpack_u8 read8()
    {
        if (m_input == m_eof)
        {
            throw bad_encoded_data();
        }
        ++m_nbytes_read;
        return *m_input++;
    }

    agbpack_u16 read16()
    {
        agbpack_u16 result = read8();
        result += read8() * 256;
        return result;
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
    agbpack_u32 m_nbytes_read = 0;
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

    void write16(agbpack_u16 word)
    {
        write8(word & 255);
        write8((word >> 8) & 255);
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
    rle = 3,
    delta = 8
};

enum class delta_options
{
    delta8 = 1,
    delta16 = 2
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

template <typename InputIterator>
void static_assert_input_type(InputIterator& input)
{
    static_assert(
        std::is_same_v<std::remove_cv_t<std::remove_reference_t<decltype(*input)>>,
        agbpack_io_datatype>,
        "Input iterator should read values of type unsigned char");
}

}

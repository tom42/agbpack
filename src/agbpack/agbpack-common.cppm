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

struct size8_tag { using type = agbpack_u8; };
struct size16_tag { using type = agbpack_u16; };
constexpr size8_tag size8;
constexpr size16_tag size16;

// TODO: align API with byte_writer, where most functions are free functions that work with other class templates too
template <std::input_iterator InputIterator>
class byte_reader final
{
public:
    byte_reader(const byte_reader&) = delete;
    byte_reader& operator=(const byte_reader&) = delete;

    explicit byte_reader(InputIterator input, InputIterator eof)
        : m_input(input)
        , m_eof(eof)
    {}

    bool eof() const
    {
        return m_input == m_eof;
    }

    agbpack_u32 nbytes_read() const
    {
        return m_nbytes_read;
    }

    agbpack_u8 read8()
    {
        if (eof())
        {
            throw bad_encoded_data();
        }
        ++m_nbytes_read;
        return *m_input++;
    }

    template <typename OutputIterator>
    void read8(std::size_t nbytes, OutputIterator output)
    {
        while (nbytes--)
        {
            *output++ = read8();
        }
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

    auto read(size8_tag) { return read8(); }

    auto read(size16_tag) { return read16(); }

private:
    agbpack_u32 m_nbytes_read = 0;
    InputIterator m_input;
    InputIterator m_eof;
};

template <typename ByteReader>
void parse_padding_bytes(ByteReader& reader)
{
    while ((reader.nbytes_read() % 4) != 0)
    {
        reader.read8();
    }
}

template <typename OutputIterator>
class byte_writer final
{
public:
    byte_writer(const byte_writer&) = delete;
    byte_writer& operator=(const byte_writer&) = delete;

    explicit byte_writer(agbpack_u32 nbytes_to_write, OutputIterator output)
        : m_nbytes_to_write(nbytes_to_write)
        , m_output(output)
    {}

    bool done() const
    {
        return m_nbytes_written >= m_nbytes_to_write;
    }

    agbpack_u32 nbytes_written() const
    {
        return m_nbytes_written;
    }

    void write8(agbpack_u8 byte)
    {
        if (done())
        {
            throw bad_encoded_data();
        }

        ++m_nbytes_written;
        *m_output++ = byte;
    }

private:
    agbpack_u32 m_nbytes_to_write;
    agbpack_u32 m_nbytes_written = 0;
    OutputIterator m_output;
};

template <typename ByteWriter>
void write8(ByteWriter& writer, agbpack_u8 byte)
{
    writer.write8(byte);
}

template <typename ByteWriter>
void write16(ByteWriter& writer, agbpack_u16 hword)
{
    write8(writer, hword & 255);
    write8(writer, (hword >> 8) & 255);
}

template <typename ByteWriter>
void write32(ByteWriter& writer, agbpack_u32 word)
{
    write8(writer, word & 255);
    write8(writer, (word >> 8) & 255);
    write8(writer, (word >> 16) & 255);
    write8(writer, (word >> 24) & 255);
}

template <typename ByteWriter>
void write(ByteWriter& writer, size8_tag, agbpack_u8 byte)
{
    write8(writer, byte);
}

template <typename ByteWriter>
void write(ByteWriter& writer, size16_tag, agbpack_u16 word)
{
    write16(writer, word);
}

template <typename ByteWriter, std::input_iterator InputIterator>
void write(ByteWriter& writer, InputIterator start, InputIterator end)
{
    for (; start != end; ++start)
    {
        write8(writer, *start);
    }
}

template <typename ByteWriter>
void write_padding_bytes(ByteWriter& writer)
{
    while (writer.nbytes_written() % 4 != 0)
    {
        write8(writer, 0);
    }
}

template <typename InputIterator>
void static_assert_input_type(InputIterator& input)
{
    static_assert(
        std::is_same_v<std::remove_cv_t<std::remove_reference_t<decltype(*input)>>,
        agbpack_io_datatype>,
        "Input iterator should read values of type unsigned char");
}

}

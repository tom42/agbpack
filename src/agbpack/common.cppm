// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>

export module agbpack:common;
import :exceptions;

namespace agbpack
{

static_assert(
    std::numeric_limits<std::size_t>::digits >= 32,
    "size_t must be at least 32 bits wide because uncompressed data can be up to 16 MB in size");

using agbpack_io_datatype = unsigned char;
using agbpack_u8 = uint8_t;
using agbpack_u16 = uint16_t;
using agbpack_u32 = uint32_t;

struct size8_tag { using type = agbpack_u8; };
struct size16_tag { using type = agbpack_u16; };
inline constexpr size8_tag size8;
inline constexpr size16_tag size16;

AGBPACK_EXPORT_FOR_UNIT_TESTING
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
            throw decode_exception();
        }

        return read8_internal();
    }

    agbpack_u8 peek8()
    {
        assert(!eof());
        return *m_input;
    }

private:
    agbpack_u8 read8_internal()
    {
        agbpack_u8 byte = *m_input++;
        ++m_nbytes_read;
        return byte;
    }

    agbpack_u32 m_nbytes_read = 0;
    InputIterator m_input;
    InputIterator m_eof;
};

template <typename ByteReader>
agbpack_u8 read8(ByteReader& reader)
{
    return reader.read8();
}

template <typename ByteReader, typename OutputIterator>
void read8(ByteReader& reader, std::size_t nbytes, OutputIterator output)
{
    while (nbytes--)
    {
        *output++ = read8(reader);
    }
}

template <typename ByteReader>
agbpack_u16 read16(ByteReader& reader)
{
    agbpack_u16 result = read8(reader);
    result += read8(reader) * 256;
    return result;
}

template <typename ByteReader>
agbpack_u32 read32(ByteReader& reader)
{
    agbpack_u32 result = read8(reader);
    result += read8(reader) * 256u;
    result += read8(reader) * 256u * 256u;
    result += read8(reader) * 256u * 256u * 256u;
    return result;
}

template <typename ByteReader>
auto read(ByteReader& reader, size8_tag)
{
    return read8(reader);
}

template <typename ByteReader>
auto read(ByteReader& reader, size16_tag)
{
    return read16(reader);
}

template <typename ByteReader>
void parse_padding_bytes(ByteReader& reader)
{
    while ((reader.nbytes_read() % 4) != 0)
    {
        read8(reader);
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
            throw decode_exception();
        }

        write8_internal(byte);
    }

private:
    void write8_internal(agbpack_u8 byte)
    {
        *m_output++ = byte;
        ++m_nbytes_written;
    }

    agbpack_u32 m_nbytes_to_write;
    agbpack_u32 m_nbytes_written = 0;
    OutputIterator m_output;
};

AGBPACK_EXPORT_FOR_UNIT_TESTING
template <typename OutputIterator>
class unbounded_byte_writer final
{
public:
    unbounded_byte_writer(const unbounded_byte_writer&) = delete;
    unbounded_byte_writer& operator=(const unbounded_byte_writer&) = delete;

    explicit unbounded_byte_writer(OutputIterator output) : m_output(output) {}

    agbpack_u32 nbytes_written() const
    {
        return m_nbytes_written;
    }

    void write8(agbpack_u8 byte)
    {
        *m_output++ = byte;
        ++m_nbytes_written;
    }

private:
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
void static_assert_input_type()
{
    using input_element_type = typename std::iterator_traits<InputIterator>::value_type;
    static_assert(
        std::is_same_v<std::remove_cv_t<std::remove_reference_t<input_element_type>>, agbpack_io_datatype>,
        "Input iterator should read values of type unsigned char");
}

bool in_closed_range(std::unsigned_integral auto x, std::unsigned_integral auto min, std::unsigned_integral auto max)
{
    return (min <= x) && (x <= max);
}

bool in_open_range(std::unsigned_integral auto x, std::unsigned_integral auto min, std::unsigned_integral auto max)
{
    return (min <= x) && (x < max);
}

template <std::unsigned_integral UnsignedIntegral>
auto make_signed(UnsignedIntegral value)
{
    return static_cast<std::make_signed_t<UnsignedIntegral>>(value);
}

}

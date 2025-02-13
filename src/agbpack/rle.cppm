// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <cassert>
#include <iterator>
#include <vector>

export module agbpack:rle;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

inline constexpr auto min_literal_run_length = 1;
inline constexpr auto max_literal_run_length = 0x80;
inline constexpr auto min_repeated_run_length = 3;
inline constexpr auto max_repeated_run_length = 0x82;
inline constexpr auto run_type_mask = 0x80;
inline constexpr auto run_length_mask = 127;

export class rle_decoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type<InputIterator>();

        byte_reader<InputIterator> reader(input, eof);
        auto header = header::parse_for_type(compression_type::rle, read32(reader));
        if (!header)
        {
            throw decode_exception();
        }

        byte_writer<OutputIterator> writer(header->uncompressed_size(), output);
        while (!writer.done())
        {
            auto flag = read8(reader);
            if (flag & run_type_mask)
            {
                agbpack_u32 n = (flag & run_length_mask) + min_repeated_run_length;
                auto byte = read8(reader);
                while (n--)
                {
                    write8(writer, byte);
                }
            }
            else
            {
                agbpack_u32 n = (flag & run_length_mask) + min_literal_run_length;
                while (n--)
                {
                    write8(writer, read8(reader));
                }
            }
        }

        parse_padding_bytes(reader);
    }
};

class literal_buffer final
{
public:
    explicit literal_buffer()
    {
        m_buffer.reserve(max_literal_run_length);
    }

    auto size()
    {
        return m_buffer.size();
    }

    void add(agbpack_u8 literal)
    {
        assert(size() < max_literal_run_length);
        m_buffer.push_back(literal);
    }

    template <typename TByteWriter>
    void flush_if_not_empty(TByteWriter& writer)
    {
        if (!m_buffer.empty())
        {
            writer.write8(static_cast<agbpack_u8>(m_buffer.size() - min_literal_run_length));
            write(writer, m_buffer.begin(), m_buffer.end());
            m_buffer.clear();
        }
    }

private:
    std::vector<agbpack_u8> m_buffer;
};

export class rle_encoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void encode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type<InputIterator>();

        // We have to encode to a temporary buffer first, because
        // * We don't know yet how many bytes of input there are, so we don't know the header content yet
        // * If the output iterator does not provide random access we cannot output encoded data first and fix up the header last
        std::vector<agbpack_u8> tmp;
        auto uncompressed_size = encode_internal(input, eof, back_inserter(tmp));

        auto header = header::create(rle_options::reserved, uncompressed_size);

        // Copy header and encoded data to output
        unbounded_byte_writer<OutputIterator> writer(output);
        write32(writer, header.to_uint32_t());
        write(writer, tmp.begin(), tmp.end());
    }

private:
    template <typename InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    agbpack_u32 encode_internal(InputIterator input, InputIterator eof, OutputIterator output)
    {
        literal_buffer literal_buffer;
        byte_reader<InputIterator> reader(input, eof);
        unbounded_byte_writer<OutputIterator> writer(output);

        while (!reader.eof())
        {
            // Find longest run of repeated bytes, but not longer than the maximum repeated run length.
            auto byte = reader.read8();
            int run_length = 1;
            while (!reader.eof() && (run_length < max_repeated_run_length) && (reader.peek8() == byte))
            {
                reader.read8();
                ++run_length;
            }

            if (run_length < min_repeated_run_length)
            {
                // Add anything too short to be encoded as a repeated run to the literal buffer.
                // If the literal buffer is full, write out a literal run of maximum length.
                for (int i = 0; i < run_length; ++i)
                {
                    literal_buffer.add(byte);
                    if (literal_buffer.size() == max_literal_run_length)
                    {
                        literal_buffer.flush_if_not_empty(writer);
                    }
                }
            }
            else
            {
                // Encode repeated run.
                // There may still be buffered literals in the literal buffer, so flush that first.
                assert((min_repeated_run_length <= run_length) && (run_length <= max_repeated_run_length));
                literal_buffer.flush_if_not_empty(writer);
                writer.write8(static_cast<agbpack_u8>(run_type_mask | (run_length - min_repeated_run_length)));
                writer.write8(byte);
            }
        }

        literal_buffer.flush_if_not_empty(writer);

        write_padding_bytes(writer);
        return reader.nbytes_read();
    }
};

}

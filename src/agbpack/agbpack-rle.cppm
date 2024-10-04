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
        static_assert_input_type(input);

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

    void add(agbpack_u8 literal)
    {
        assert(size() < max_literal_run_length);
        m_buffer.push_back(literal);
    }

    // TODO: for whatever inane reason I cannot use an std::outputiterator thing here. Now why again? Da fuk?
    template <typename TByteWriter>
    void flush(TByteWriter& writer)
    {
        writer.write8(static_cast<agbpack_u8>(m_buffer.size() - min_literal_run_length));
        write(writer, m_buffer.begin(), m_buffer.end());
        m_buffer.clear();
    }

    auto size()
    {
        return m_buffer.size();
    }

    auto empty()
    {
        return m_buffer.empty();
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
        static_assert_input_type(input);

        // We have to encode to a temporary buffer first, because
        // * We don't know yet how many bytes of input there are, so we don't know the header content yet
        // * If the output iterator does not provide random access we cannot output encoded data first and fix up the header last
        std::vector<agbpack_u8> tmp;
        auto uncompressed_size = encode_internal(input, eof, back_inserter(tmp));

        // TODO: verify uncompressed size and throw appropriate exception!
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

        // TODO: implement (see delta_encoder)
        //       * Encode to tmp buffer
        //       * Add padding bytes (that's required, innit?)
        //       * Create and write header to output => good place to start: we cannot create a header.
        //       * Copy tmp to output

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

            // TODO: next: maybe try encoding some literal runs first? These need less special handling because there is no minimum run length, only a maximum one.
            //             => Basically we can just ebery loop iteration add a literal to the literal buffer
            //             => If the literal buffer is full we flush it (that is, we write a maximum run)
            //             => After the loop we need to check whether there is still data in the literal buffer. If so we need to flush it. Simple? Simple.
            if (run_length < min_repeated_run_length)
            {
                // Add anything too short to be encoded as a repeated run to the literal buffer.
                // If the literal buffer is full, write out a literal run of maximum length.
                for (int i = 0; i < run_length; ++i)
                {
                    literal_buffer.add(byte);
                    if (literal_buffer.size() == max_literal_run_length)
                    {
                        literal_buffer.flush(writer);
                    }
                }
            }
            else
            {
                // TODO: transition from run to no run still missing. That is, flush literal buffer here if it is not empty
                assert((min_repeated_run_length <= run_length) && (run_length <= max_repeated_run_length));
                writer.write8(static_cast<agbpack_u8>(run_type_mask | (run_length - min_repeated_run_length)));
                writer.write8(byte);
            }
        }

        if (!literal_buffer.empty())
        {
            literal_buffer.flush(writer);
        }

        write_padding_bytes(writer);
        return reader.nbytes_read();
    }
};

}

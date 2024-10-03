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

// TODO: types? do we care?
// TODO: should these not be inline?
constexpr auto min_literal_run_length = 1;
constexpr auto max_literal_run_length = 0x80;
constexpr auto min_repeated_run_length = 3;
constexpr auto max_repeated_run_length = 0x82;

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
            if (flag & 0x80)
            {
                agbpack_u32 n = (flag & 127) + min_repeated_run_length;
                auto byte = read8(reader);
                while (n--)
                {
                    write8(writer, byte);
                }
            }
            else
            {
                agbpack_u32 n = (flag & 127) + min_literal_run_length;
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
        std::vector<agbpack_u8> literal_buffer; // TODO: replace this completely by class literal_buffer
        byte_reader<InputIterator> reader(input, eof);
        unbounded_byte_writer<OutputIterator> writer(output);

        // TODO: implement (see delta_encoder)
        //       * Encode to tmp buffer
        //       * Add padding bytes (that's required, innit?)
        //       * Create and write header to output => good place to start: we cannot create a header.
        //       * Copy tmp to output

        while (!reader.eof())
        {
            // TODO: find longest run => own function?
            // TODO: honor maximum repeated run length here!
            auto byte = reader.read8();
            int run_length = 1;
            while (!reader.eof() && (reader.peek8() == byte))
            {
                reader.read8();
                ++run_length;
            }

            // TODO: next: maybe try encoding some literal runs first? These need less special handling because there is no minimum run length, only a maximum one.
            //             => Basically we can just ebery loop iteration add a literal to the literal buffer
            //             => If the literal buffer is full we flush it (that is, we write a maximum run)
            //             => After the loop we need to check whether there is still data in the literal buffer. If so we need to flush it. Simple? Simple.
            // TODO: need special handling of runs < min_repeated_run_length
            if (run_length < min_repeated_run_length) // TODO: temporary hack to get scanning/encoding of runs correct (transition from run to no run still missing)
            {
                // TODO: still not correct: we need to check whether to flush the literal buffer for each byte that we add
                for (int i = 0; i < run_length; ++i)
                {
                    literal_buffer.push_back(byte);
                }

                if (literal_buffer.size() == max_literal_run_length) // TODO: == or >= ?
                {
                    // TODO: same code as below: factor out!
                    writer.write8(static_cast<agbpack_u8>(literal_buffer.size() - min_literal_run_length));
                    write(writer, literal_buffer.begin(), literal_buffer.end());
                    literal_buffer.clear();
                }
            }
            else
            {
                // TODO: unhardcode 0x80
                assert((min_repeated_run_length <= run_length) && (run_length <= max_repeated_run_length));
                writer.write8(static_cast<agbpack_u8>(0x80 | (run_length - min_repeated_run_length)));
                writer.write8(byte);
            }
        }

        if (!literal_buffer.empty())
        {
            // TODO: flush literal buffer (that's so not the real implementation, but good enough to pass the next test)
            // TODO: assert max. literal run? => Nah assertion should be done prior to push_back to literal buffer, no?
            // TODO: consider reusing this code inside the main encoding loop. If we do so then we need to clear the literal buffer, though!
            writer.write8(static_cast<agbpack_u8>(literal_buffer.size() - min_literal_run_length));
            write(writer, literal_buffer.begin(), literal_buffer.end());
            literal_buffer.clear();
        }

        write_padding_bytes(writer);
        return reader.nbytes_read();
    }
};

}

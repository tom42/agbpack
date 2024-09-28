// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>
#include <vector>

export module agbpack:rle;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

// TODO: type? do we care?
// TODO: should this not be inline?
constexpr auto max_literal_run_length = 0x80;

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
                agbpack_u32 n = (flag & 127) + 3;
                auto byte = read8(reader);
                while (n--)
                {
                    write8(writer, byte);
                }
            }
            else
            {
                agbpack_u32 n = (flag & 127) + 1;
                while (n--)
                {
                    write8(writer, read8(reader));
                }
            }
        }

        parse_padding_bytes(reader);
    }
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
        std::vector<agbpack_u8> literal_buffer;
        literal_buffer.reserve(max_literal_run_length);
        byte_reader<InputIterator> reader(input, eof);

        while (!reader.eof())
        {
            // TODO: danger: infinite loop: this works only for empty inputs.
            // TODO: next: maybe try encoding some literal runs first? These need less special handling because there is no minimum run length, only a maximum one.
            //             => Basically we can just ebery loop iteration add a literal to the literal buffer
            //             => If the literal buffer is full we flush it (that is, we write a maximum run)
            //             => After the loop we need to check whether there is still data in the literal buffer. If so we need to flush it. Simple? Simple.
        }

        // TODO: implement (see delta_encoder)
        //       * Encode to tmp buffer
        //       * Add padding bytes (that's required, innit?)
        //       * Create and write header to output => good place to start: we cannot create a header.
        //       * Copy tmp to output

        // TODO: verify uncompressed size and throw appropriate exception!
        auto header = header::create(rle_options::reserved, 0); // TODO: unhardcode uncompressed size

        // Copy header and encoded data to output
        unbounded_byte_writer<OutputIterator> writer(output);
        write32(writer, header.to_uint32_t());
        write(writer, tmp.begin(), tmp.end());
    }
};

}

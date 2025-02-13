// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>
#include <stdexcept>
#include <vector>

export module agbpack:delta;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

export class delta_decoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type<InputIterator>();

        byte_reader<InputIterator> reader(input, eof);
        auto header = header::parse_for_type(compression_type::delta, read32(reader));
        if (!header)
        {
            throw decode_exception();
        }

        byte_writer<OutputIterator> writer(header->uncompressed_size(), output);
        decode8or16(header->template options_as<delta_options>(), reader, writer);
    }

private:
    template <typename InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    static void decode8or16(delta_options options, byte_reader<InputIterator>& reader, byte_writer<OutputIterator>& writer)
    {
        switch (options)
        {
            case delta_options::delta8:
                generic_decode(size8, reader, writer);
                return;
            case delta_options::delta16:
                generic_decode(size16, reader, writer);
                return;
        }

        throw decode_exception();
    }

    template <typename SizeTag, typename InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    static void generic_decode(SizeTag, byte_reader<InputIterator>& reader, byte_writer<OutputIterator>& writer)
    {
        typename SizeTag::type current_value = 0;

        while (!writer.done())
        {
            current_value += read(reader, SizeTag());
            write(writer, SizeTag(), current_value);
        }

        parse_padding_bytes(reader);
    }
};

export class delta_encoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void encode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        // TODO: this try/catch block is a hack to get our tests passing. We really need to redesign exception handling
        try
        {
            static_assert_input_type<InputIterator>();

            // We have to encode to a temporary buffer first, because
            // * We don't know yet how many bytes of input there are, so we don't know the header content yet
            // * If the output iterator does not provide random access we cannot output encoded data first and fix up the header last
            std::vector<agbpack_u8> tmp;
            auto uncompressed_size = encode8or16(input, eof, back_inserter(tmp));

            auto header = header::create(m_options, uncompressed_size);

            // Copy header and encoded data to output
            unbounded_byte_writer<OutputIterator> writer(output);
            write32(writer, header.to_uint32_t());
            write(writer, tmp.begin(), tmp.end());
        }
        catch (const decode_exception&)
        {
            throw encode_exception("input must contain an even number of bytes for 16 bit delta encoding");
        }
    }

    void options(delta_options options)
    {
        if (!is_valid(options))
        {
            throw std::invalid_argument("invalid delta compression options");
        }

        m_options = options;
    }

private:
    template <typename InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    agbpack_u32 encode8or16(InputIterator input, InputIterator eof, OutputIterator output)
    {
        switch (m_options)
        {
            case delta_options::delta8:
                return generic_encode(size8, input, eof, output);
            case delta_options::delta16:
                return generic_encode(size16, input, eof, output);
        }

        throw internal_error("invalid delta compression options, but this line should never be reached");
    }

    template <typename SizeTag, typename InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    agbpack_u32 generic_encode(SizeTag, InputIterator input, InputIterator eof, OutputIterator output)
    {
        using symbol_type = typename SizeTag::type;

        std::vector<agbpack_u8> encoded_data;
        byte_reader<InputIterator> reader(input, eof);
        unbounded_byte_writer<OutputIterator> writer(output);

        symbol_type old_value = 0;
        while (!reader.eof())
        {
            symbol_type current_value = read(reader, SizeTag());
            symbol_type delta = current_value - old_value;
            old_value = current_value;
            write(writer, SizeTag(), delta);
        }

        write_padding_bytes(writer);
        return reader.nbytes_read();
    }

    delta_options m_options = delta_options::delta8;
};

}

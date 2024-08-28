// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>

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
        static_assert_input_type(input);

        byte_reader<InputIterator> reader(input, eof);
        auto header = header::parse_for_type(compression_type::delta, reader.read32());
        if (!header)
        {
            throw bad_encoded_data();
        }

        if (header->uncompressed_size())
        {
            byte_writer<OutputIterator> writer(header->uncompressed_size(), output);
            decode8or16(header->template options_as<delta_options>(), reader, writer);
        }
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

        throw bad_encoded_data();
    }

    template <typename SizeTag, typename InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    static void generic_decode(SizeTag symbol_size, byte_reader<InputIterator>& reader, byte_writer<OutputIterator>& writer)
    {
        auto current_value = reader.read(symbol_size);
        writer.write(symbol_size, current_value);

        while (!writer.done())
        {
            current_value += reader.read(symbol_size);
            writer.write(symbol_size, current_value);
        }

        reader.parse_padding_bytes();
    }
};

export class delta_encoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void encode(InputIterator input, InputIterator /*eof*/, OutputIterator output)
    {
        static_assert_input_type(input);
        // TODO: encode to temporary buffer
        // TODO: do we want to optimize for RandomAccessIterator?
        // TODO: write to output
        //       * For starters we can probably use the existing byte_writer, but maybe we want to have an unchecked variant?
        //       * Yeah but in the spirit of C++ you'd then make many of the write functions non-element functions, no?

        // TODO: unhardcode data
        // TODO: need a way to create headers (as opposed to parsing headers)
        // TODO: what if data is too big to fit into a compression header? We should test this, no?
        byte_writer<OutputIterator> writer(4, output);
        writer.write8(0x81);
        writer.write8(0);
        writer.write8(0);
        writer.write8(0);
    }
};

}

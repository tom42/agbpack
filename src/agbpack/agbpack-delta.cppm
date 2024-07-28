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

}

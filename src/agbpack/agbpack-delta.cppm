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
    template <std::input_iterator InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type(input);

        // TODO: forbid copying byte_reader?
        // TODO: forbid copying byte_writer?
        byte_reader<InputIterator> reader(input, eof);
        auto maybe_header = delta_header::parse(reader.read32());
        if (!maybe_header)
        {
            throw bad_encoded_data();
        }

        auto header = maybe_header.value(); // TODO: not pretty. can we get rid of it?

        if (header.uncompressed_size())
        {
            byte_writer<OutputIterator> writer(header.uncompressed_size(), output);
            decode8or16(header, reader, writer);
        }

        // TODO: ensure padding at end of input:
        //       * We could do this in the byte_reader dtor, but we must not throw in there. Sigh?
        //       * Well if we only want to do it once we must create the byte_reader up here and pass it to callees
    }

private:
    template <typename InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    static void decode8or16(delta_header header, byte_reader<InputIterator>& reader, byte_writer<OutputIterator>& writer)
    {
        switch (header.options())
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
    static void generic_decode(SizeTag size_tag, byte_reader<InputIterator>& reader, byte_writer<OutputIterator>& writer)
    {
        auto current_value = reader.read(size_tag);
        writer.write(size_tag, current_value);

        while (!writer.done())
        {
            current_value += reader.read(size_tag);
            writer.write(size_tag, current_value);
        }
    }
};

}

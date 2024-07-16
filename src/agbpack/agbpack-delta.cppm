// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>

export module agbpack:delta;
import :common;
import :exceptions;

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
        header header(reader.read32());

        // TODO: also check options
        // TODO: move to static method
        if (header.type() != compression_type::delta)
        {
            throw bad_encoded_data();
        }

        if (header.uncompressed_size())
        {
            // TODO: for symmetry, should we also pass a writer by reference rather than the output iterator?
            decode8or16(header, reader, output);
        }

        // TODO: ensure padding at end of input:
        //       * We could do this in the byte_reader dtor, but we must not throw in there. Sigh?
        //       * Well if we only want to do it once we must create the byte_reader up here and pass it to callees
    }

private:
    template <typename InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    static void decode8or16(header header, byte_reader<InputIterator>& reader, OutputIterator output)
    {
        // TODO: this cast is rather ugly. Should we model header as a variant type of sorts?
        switch (static_cast<delta_options>(header.options()))
        {
            case delta_options::delta8:
                decode8(header, reader, output);
                return;
            case delta_options::delta16:
                decode16(header, reader, output);
                return;
        }

        throw bad_encoded_data();
    }

    template <typename InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    static void decode8(header header, byte_reader<InputIterator>& reader, OutputIterator output)
    {
        byte_writer<OutputIterator> writer(header.uncompressed_size(), output);

        auto current_value = reader.read8();
        writer.write8(current_value);

        while (!writer.done())
        {
            current_value += reader.read8();
            writer.write8(current_value);
        }
    }

    template <typename InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    static void decode16(header header, byte_reader<InputIterator>& reader, OutputIterator output)
    {
        byte_writer<OutputIterator> writer(header.uncompressed_size(), output);

        auto current_value = reader.read16();
        writer.write16(current_value);

        while (!writer.done())
        {
            current_value += reader.read16();
            writer.write16(current_value);
        }
    }
};

}

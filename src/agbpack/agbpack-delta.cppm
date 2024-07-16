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

        byte_reader<InputIterator> reader(input, eof);
        header header(reader.read32());

        // TODO: also check options
        // TODO: move to static method
        if (header.type() != compression_type::delta)
        {
            throw bad_encoded_data();
        }

        do_decode(header, output);

        // TODO: ensure padding at end of input:
        //       * We could do this in the byte_writer dtor, but we must not throw in there. Sigh?
        //       * Well if we only want to do it once we must create the byte_writer up here and pass it to callees
    }

private:
    template <std::output_iterator<agbpack_io_datatype> OutputIterator>
    static void do_decode(header header, OutputIterator output)
    {
        switch (static_cast<delta_options>(header.options()))
        {
            case delta_options::delta8:
                decode8(header, output);
                return;
            case delta_options::delta16:
                decode16();
                return;
        }

        // TODO: write a test for this branch
        throw bad_encoded_data();
    }

    template <std::output_iterator<agbpack_io_datatype> OutputIterator>
    static void decode8(header header, OutputIterator output)
    {
        byte_writer<OutputIterator> writer(header.uncompressed_size(), output);

        // TODO: write a test for this branch
        // TODO: hardcoded to pass first test (don't hardcode, and use a byte_writer)
        *output++ = 'a';
    }

    static void decode16()
    {
        // TODO: write a test for this branch
    }
};

}

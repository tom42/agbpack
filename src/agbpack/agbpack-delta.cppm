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
    void decode(InputIterator input, InputIterator eof, OutputIterator /*output*/)
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

        do_decode(header);

        /*switch ()
        {
            case delta_options::delta8:
                // TODO: hardcoded to pass first test (don't hardcode, and use a byte_writer)
                *output++ = 'a';
                break;

        }*/


    }

private:
    static void do_decode(header header)
    {
        switch (static_cast<delta_options>(header.options()))
        {
            case delta_options::delta8:
                decode8();
                return;
            case delta_options::delta16:
                decode16();
                return;
        }

        // TODO: write a test for this branch
        throw bad_encoded_data();
    }

    static void decode8()
    {
        // TODO: write a test for this branch
    }

    static void decode16()
    {
        // TODO: write a test for this branch
    }
};

}

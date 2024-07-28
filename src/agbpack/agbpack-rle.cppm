// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>

export module agbpack:rle;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

export class rle_decoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type(input);

        byte_reader<InputIterator> reader(input, eof);
        auto header = header::parse_for_type(compression_type::rle, reader.read32());
        if (!header)
        {
            throw bad_encoded_data();
        }

        byte_writer<OutputIterator> writer(header->uncompressed_size(), output);
        while (!writer.done())
        {
            auto flag = reader.read8();
            if (flag & 0x80)
            {
                agbpack_u32 n = (flag & 127) + 3;
                auto byte = reader.read8();
                while (n--)
                {
                    writer.write8(byte);
                }
            }
            else
            {
                agbpack_u32 n = (flag & 127) + 1;
                while (n--)
                {
                    writer.write8(reader.read8());
                }
            }
        }

        reader.parse_padding_bytes();
    }
};

}

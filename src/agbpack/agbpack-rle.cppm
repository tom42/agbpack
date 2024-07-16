// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>

export module agbpack:rle;
import :common;

namespace agbpack
{

export class rle_decoder final
{
public:
    template <std::input_iterator InputIterator, std::output_iterator<agbpack_io_datatype> OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type(input);

        byte_reader<InputIterator> reader(input, eof);
        header header(reader.read32());
        verify_header(header);

        byte_writer<OutputIterator> writer(header.uncompressed_size(), output);
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

        // Require padding bytes at end of compressed data to be present.
        while ((reader.nbytes_read() % 4) != 0)
        {
            reader.read8();
        }
    }

private:
    static void verify_header(const header& header)
    {
        if (header.type() != compression_type::rle)
        {
            throw bad_encoded_data();
        }
        if (header.options() != 0)
        {
            throw bad_encoded_data();
        }
    }
};

}

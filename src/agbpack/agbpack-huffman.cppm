// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

module;

#include <iterator>

export module agbpack:huffman;
import :common;
import :exceptions;
import :header;

namespace agbpack
{

export class huffman_decoder final
{
public:
    template <std::input_iterator InputIterator, typename OutputIterator>
    void decode(InputIterator input, InputIterator eof, OutputIterator output)
    {
        static_assert_input_type(input);

        byte_reader<InputIterator> reader(input, eof);
        auto header = header::parse_for_type(compression_type::huffman, reader.read32());
        if (!header)
        {
            throw bad_encoded_data();
        }

        // TODO: actually calculate and store tree size
        // TODO: also document a bit how to interpret this?
        //       Basically it points at the bitstream?
        // TODO: probably we want to read the entire tree including the tree size into a vector...
        reader.read8();

        // TODO: read huffman tree (what sizes do we support?)

        // TODO: decode data (what sizes do we support?)
        //       * Note: Quote: "Compressed Bitstream (stored in units of 32bits)"
        //       * This means that
        //         * We need to read 32 bit units into our bit buffer
        //         * Data is automatically aligned, so there should never be any padding bytes
        //         * ??? There might be padding between the huffman tree and the bit stream ???
        *output++ = 'a';
        *output++ = 'b';

        // TODO: parse padding bytes here
    }
private:
};

}

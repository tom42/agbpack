// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <string>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

using string = std::string;

TEST_CASE("huffman_decoder_test")
{
    agbpack::huffman_decoder decoder;

    SECTION("Valid input")
    {
        string filename_part = GENERATE(
            // TODO: find a reference encoder that compresses this
            //       * GBACrusher will not work. It will produce a compressed file, but that file appears to be broken:
            //         The compressed file appears to have the tree size field, but no actual tree.
            //         The CUE decoder does not correctly decompress it and prints WARNING: unexpected end of encoded file!
            //       * The CUE encoder seems to silently not compress this file, presumably because the resulting file
            //         will obviously be bigger.
            //         => Might have to find another reference encoder, or modify CUE
            //       * Might also be useful to find out what GBACrusher's problem is:
            //         * Does the following input work: aa (I'd expect not, since it's still only one symbol)
            //         * Does the following input work: ab (It seems so, and that would hint that GBACrusher has problems
            //           if the huffman tree contains only one node)
            //"huffman.good.8.1-symbol.txt", // TODO: have not yet found an encoder that encodes a file consisting of a single letter 'a' correctly. Deleted test file for the time being
            "huffman.good.8.2-symbols.txt",
            "huffman.good.8.3-symbols.txt",
            "huffman.good.8.256-symbols.bin", // TODO: also add 4 bit variant
            "huffman.good.4.2-symbols.txt",
            "huffman.good.4.3-symbols.txt");
        auto expected_data = read_file(filename_part + ".decoded");

        auto decoded_data = decode_file(decoder, filename_part + ".encoded");

        CHECK(decoded_data == expected_data);
    }

    SECTION("Invalid input")
    {
        auto encoded_file = GENERATE(
            "huffman.bad.eof-inside-header.txt.encoded",
            "huffman.bad.eof-at-tree-size.txt.encoded",
            "huffman.bad.eof-inside-tree.txt.encoded",
            "huffman.bad.eof-while-reading-bitstream.txt.encoded",
            "huffman.bad.invalid-compression-type-in-header.txt.encoded",
            "huffman.bad.valid-but-unexpected-compression-type-in-header.txt.encoded",
            "huffman.bad.invalid-compression-options-in-header.txt.encoded");

        CHECK_THROWS_AS(decode_file(decoder, encoded_file), agbpack::bad_encoded_data);
    }
}

}

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
            // Notes on test files:
            // * Files consisting of a single byte of input have been created using GValiente's gba-huff.h,
            //   which doesn't seem to be available on github anymore.
            //   The encoded data subsequently needed manual fixing, since the uncompressed size in the header
            //   was wrong (4 bytes when it should have been 1 byte)
            // * Files consisting of zero bytes of input have been manually derived from files consisting
            //   of a single byte of input, since no reference encoder was able to encode zero bytes of input.
            //   * Since there is no uncompressed data, there is no bitstream, so it was removed manually.
            //   * Tree size byte is 1 (the minimum size). There is a root node with two bogus child nodes,
            //     each containing the symbol 0.
            "huffman.good.8.0-bytes.txt",
            "huffman.good.8.1-byte.txt",
            "huffman.good.8.2-bytes.txt",
            "huffman.good.8.3-bytes.txt",
            "huffman.good.8.256-bytes.bin",
            "huffman.good.4.0-bytes.txt",
            "huffman.good.4.1-byte.txt",
            "huffman.good.4.2-bytes.txt",
            "huffman.good.4.3-bytes.txt",
            "huffman.good.4.256-bytes.bin");
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
            "huffman.bad.invalid-compression-options-in-header.txt.encoded",
            "huffman.bad.misaligned-bitstream.txt.encoded",
            "huffman.bad.4.garbage-in-unused-bits-of-leaf-node.txt.encoded",
            "huffman.bad.8.huffman-tree-access-past-end-of-tree.txt.encoded");

        CHECK_THROWS_AS(decode_file(decoder, encoded_file), agbpack::decode_exception);
    }
}

}

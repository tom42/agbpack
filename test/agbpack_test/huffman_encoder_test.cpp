// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <iostream> // TODO: remove
#include <stdexcept>
#include <string>
#include "testdata.hpp"

import agbpack;

namespace agbpack_test
{

using string = std::string;

// TODO: add test: input data too big
TEST_CASE("huffman_encoder_test")
{
    agbpack::huffman_encoder encoder;

    SECTION("Successful 8 bit encoding with reference encoded data")
    {
        // TODO: add necessary tests:
        //       - 1 byte
        //       - 2 byte
        const string filename_part = GENERATE(
            "huffman.good.8.0-bytes.txt");
        const auto expected_data = read_file(filename_part + ".encoded");

        encoder.options(agbpack::huffman_options::h8);
        const auto encoded_data = encode_file(encoder, filename_part + ".decoded");

        CHECK(encoded_data == expected_data);
    }

    SECTION("Successful 4 bit encoding with reference encoded data")
    {
        // TODO: add necessary tests (see 8 bit counterpart)
        const string filename_part = GENERATE(
            "huffman.good.4.0-bytes.txt");
        const auto expected_data = read_file(filename_part + ".encoded");

        encoder.options(agbpack::huffman_options::h4);
        const auto encoded_data = encode_file(encoder, filename_part + ".decoded");

        CHECK(encoded_data == expected_data);
    }

    SECTION("Successful 8 bit encoding with check against decoder")
    {
        const string filename = GENERATE(
            "huffman.good.8.0-bytes.txt.decoded",
            "huffman.good.frequency-table-test.txt.decoded");
        const auto original_data = read_file(filename);

        // Encode
        encoder.options(agbpack::huffman_options::h8);
        const auto encoded_data = encode_vector(encoder, original_data);

        // Decode
        agbpack::huffman_decoder decoder;
        const auto decoded_data = decode_vector(decoder, encoded_data);

        CHECK(decoded_data == original_data);
    }

    SECTION("Successful 4 bit encoding with check against decoder")
    {
        const string filename = GENERATE(
            "huffman.good.4.0-bytes.txt.decoded",
            "huffman.good.frequency-table-test.txt.decoded");
        const auto original_data = read_file(filename);

        // Encode
        encoder.options(agbpack::huffman_options::h4);
        const auto encoded_data = encode_vector(encoder, original_data);

        // Decode
        agbpack::huffman_decoder decoder;
        const auto decoded_data = decode_vector(decoder, encoded_data);

        CHECK(decoded_data == original_data);
    }

    SECTION("Invalid options")
    {
        CHECK_THROWS_MATCHES(
            encoder.options(agbpack::huffman_options(-1)),
            std::invalid_argument,
            Catch::Matchers::Message("invalid huffman compression options"));
    }
}

// TODO: for debugging, remove
TEST_CASE("zzz_test")
{
    // TODO: Implement test
    //       * get some test data
    //         * Foo?
    //         * Some sort of lucas sequence number thing? => Maybe the latter
    //       * Create reference encoded data
    //       * Dump codes
    //       * Create codes using our tree building mechanism
    //       * Does it print the same output?
    //
    // So, here are the foo codes as dumped while decoding:
    //   o: 00
    //   s: 100
    //    : 111
    //   h: 0100
    //   l: 0101
    //   w: 0110
    //   e: 0111
    //   f: 1010
    //   t: 1011
    //   .: 11000
    //   a: 11010
    //   H: 11001
    //   b: 11011
    //
    // TODO: see whether we have any other use for the foo file
    agbpack::huffman_decoder decoder;
    decode_file(decoder, "huffman.good.8.foo.txt.encoded");

    std::cout << "meh\n";
}

}

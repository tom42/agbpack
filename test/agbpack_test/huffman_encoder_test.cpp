// SPDX-FileCopyrightText: 2024 Thomas Mathys
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
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
        // TODO: have dedicated test input that requires padding of the bitstream (huffman.good.frequency-table-test.txt.decoded does this, but it has a bad name)
        // TODO: have dedicated test input that requires flushing of the bitstream (huffman.good.frequency-table-test.txt.decoded does this too, but it really has a bad name)
        const string filename = GENERATE(
            "huffman.good.8.0-bytes.txt.decoded",
            "huffman.good.frequency-table-test.txt.decoded"); // TODO: this fails. figure out why
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
            "huffman.good.4.0-bytes.txt.decoded");
            //"huffman.good.frequency-table-test.txt.decoded"); // TODO: temporarily disabled. Enable again when we can serialize trees ourselves
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

// TODO: for development/debugging, remove
TEST_CASE("zzz_test")
{
    // TODO: Get this test running:
    //       * In the production code, hardcode a tree we obtained from CUE in some way
    //       * Copy that tree to the compressed output
    //       * Use that tree to encode the output
    //       * Do this both for
    //         * 8 bit
    //         * 4 bit
    //         * This gives us an encoder that is complete except for tree serialization, which we can then tackle last
    // TODO: Implement test
    //       * get some test data
    //         * Foo?
    //         * Some sort of lucas sequence number thing? => Maybe the latter
    //         * Another test: equally distributed symbols with 8 and 4 bit encoding
    //       * Create reference encoded data
    //       * Dump codes
    //       * Create codes using our tree building mechanism
    //       * Does it print the same output?
    // TODO: see whether we have any other use for the following files (e.g. use them in some test)
    //       * huffman.good.8.foo.txt
    //       * huffman.good.8.helloworld.txt
    //
    const string basename = "huffman.good.8.helloworld.txt";
    const auto original_data = read_file(basename + ".decoded");

    // Encode data
    agbpack::huffman_encoder encoder;
    encoder.options(agbpack::huffman_options::h8);
    const auto encoded_data = encode_vector(encoder, original_data);

    // Decode data: check whether our decoder can read the data produced by our encoder
    agbpack::huffman_decoder decoder;
    const auto decoded_data = decode_vector(decoder, encoded_data);
    CHECK(decoded_data == original_data);
}

}
